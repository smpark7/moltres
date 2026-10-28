#include "XsTable.h"
#include "MooseError.h"

#include "libmesh/dense_matrix.h"
#include "libmesh/dense_vector.h"

#include <algorithm>
#include <cmath>
#include <functional>
#include <limits>

// On one axis, the interpolants reproduce MOOSE's
// LinearInterpolation, SplineInterpolation (natural end conditions) and MonotoneCubicInterpolation
// (Fritsch-Carlson) operation for operation, so results match those classes bit for bit. Keep the
// expression order when editing them.

namespace
{

/// Constant: the values of the first branch, zero derivative
class XsNoneBasis : public XsBasis
{
public:

  void evaluate(const Real * /*x*/, unsigned int /*d_axis*/, Real * values, Real * dvalues) const override
  {
    std::copy(_y.begin(), _y.begin() + _n, values);
    if (dvalues)
      std::fill(dvalues, dvalues + _n, 0.);
  }
};

/**
 * Multilinear on any number of axes. The cell is reduced one axis at a time with the 1-D formula
 * of LinearInterpolation, the derivative axis last, so that a single axis reproduces
 * LinearInterpolation exactly. Constant outside the grid.
 */
class XsMultilinearBasis : public XsBasis
{
public:

  void setData(const XsAxes & axes, unsigned int n, std::vector<Real> data) override
  {
    XsBasis::setData(axes, n, std::move(data));

    const auto D = _axes.size();
    _stride.assign(D, 1);
    for (auto a = D; a-- > 1;)
      _stride[a - 1] = _stride[a] * _axes[a].size();
    _t.resize(D);
    _dx.resize(D);
    _active.reserve(D);
    _corners.reserve(1u << D);
    _buf.resize((1u << D) / 2 * _n);
  }

  void evaluate(const Real * x, unsigned int d_axis, Real * values, Real * dvalues) const override
  {
    const auto D = _axes.size();
    _active.clear();
    bool d_active = false;
    std::size_t base = 0;
    for (std::size_t a = 0; a < D; ++a)
    {
      const auto & g = _axes[a];
      const auto L = g.size();
      if (std::isnan(x[a]))
      {
        std::fill(values, values + _n, std::nan(""));
        if (dvalues)
          std::fill(dvalues, dvalues + _n, std::nan(""));
        return;
      }

      // Either a single knot (no interval) or the interval [lo, lo + 1] with offset t from lo
      bool span = true;
      std::size_t lo;
      if (L == 1)
      {
        lo = 0;
        span = false;
      }
      else if (x[a] < g[0])
      {
        lo = 0;
        span = false;
      }
      else if (x[a] >= g.back())
      {
        lo = L - 1;
        span = false;
      }
      else
        lo = std::distance(g.begin(), std::upper_bound(g.begin(), g.end(), x[a])) - 1;

      base += lo * _stride[a];
      if (span)
      {
        _t[a] = x[a] - g[lo];
        _dx[a] = g[lo + 1] - g[lo];
        if (a == d_axis)
          d_active = true;
        else
          _active.push_back(a);
      }
    }
    // The derivative axis is reduced last
    if (d_active)
      _active.push_back(d_axis);

    const auto k = _active.size();
    if (k == 0)
    {
      std::copy_n(&_y[base * _n], _n, values);
      if (dvalues)
        std::fill(dvalues, dvalues + _n, 0.);
      return;
    }

    // Branch offsets of the cell corners; bit i of the corner index selects the upper knot of
    // axis _active[i]
    const std::size_t m = std::size_t(1) << k;
    _corners.assign(m, base);
    for (std::size_t j = 0; j < m; ++j)
      for (std::size_t i = 0; i < k; ++i)
        if (j >> i & 1)
          _corners[j] += _stride[_active[i]];

    // Reduce along _active[0], reading the corners from the table, then along the remaining
    // axes in place; the last reduction writes the result
    for (std::size_t i = 0; i < k; ++i)
    {
      const auto a = _active[i];
      const Real t = _t[a];
      const Real dx = _dx[a];
      const std::size_t pairs = m >> (i + 1);
      const bool last = i + 1 == k;
      for (std::size_t j = 0; j < pairs; ++j)
      {
        const Real * y0 = i == 0 ? &_y[_corners[2 * j] * _n] : &_buf[2 * j * _n];
        const Real * y1 = i == 0 ? &_y[_corners[2 * j + 1] * _n] : &_buf[(2 * j + 1) * _n];
        Real * out = last ? values : &_buf[j * _n];
        if (last && dvalues)
        {
          if (d_active)
            for (unsigned int c = 0; c < _n; ++c)
              dvalues[c] = (y1[c] - y0[c]) / dx;
          else
            std::fill(dvalues, dvalues + _n, 0.);
        }
        for (unsigned int c = 0; c < _n; ++c)
          out[c] = y0[c] + (y1[c] - y0[c]) * t / dx;
      }
    }
  }

private:
  /// Branch-index stride of each axis
  std::vector<std::size_t> _stride;

  /// Scratch for evaluate()
  mutable std::vector<Real> _t;
  mutable std::vector<Real> _dx;
  mutable std::vector<std::size_t> _active;
  mutable std::vector<std::size_t> _corners;
  mutable std::vector<Real> _buf;
};

/**
 * Polynomial of a given total degree in the state variables, fitted to the tabulated values in
 * the least-squares sense. The coordinates are scaled to [-1, 1] on each axis for conditioning,
 * axes with a single knot are left out, and the state point is held inside the table range.
 */
class XsLeastSquaresBasis : public XsBasis
{
public:
  XsLeastSquaresBasis(unsigned int order) : _order(order) {}

  void setData(const XsAxes & axes, unsigned int n, std::vector<Real> data) override
  {
    XsBasis::setData(axes, n, std::move(data));
    const auto D = _axes.size();

    _active.clear();
    _center.assign(D, 0.);
    _half.assign(D, 1.);
    for (std::size_t a = 0; a < D; ++a)
    {
      const auto & g = _axes[a];
      if (g.size() == 1)
        continue;
      if (g.size() < _order + 1)
        mooseError("A least-squares fit of order ", _order, " needs at least ", _order + 1,
                   " knots on every axis that has more than one; axis ", a, " has ", g.size(),
                   ".");
      _active.push_back(a);
      _center[a] = 0.5 * (g.front() + g.back());
      _half[a] = 0.5 * (g.back() - g.front());
    }

    // Exponents of each term on the active axes, total degree up to _order
    _terms.clear();
    std::vector<unsigned int> e(_active.size(), 0);
    const std::function<void(std::size_t, unsigned int)> enumerate =
        [&](std::size_t i, unsigned int left)
    {
      if (i == _active.size())
      {
        _terms.push_back(e);
        return;
      }
      for (unsigned int p = 0; p <= left; ++p)
      {
        e[i] = p;
        enumerate(i + 1, left - p);
      }
      e[i] = 0;
    };
    enumerate(0, _order);

    // Design matrix: the terms at every branch
    std::vector<std::size_t> stride(D, 1);
    for (auto a = D; a-- > 1;)
      stride[a - 1] = stride[a] * _axes[a].size();
    const std::size_t B = _y.size() / _n;
    const std::size_t T = _terms.size();
    libMesh::DenseMatrix<Real> A(B, T);
    std::vector<Real> t(D);
    for (std::size_t b = 0; b < B; ++b)
    {
      for (std::size_t a = 0; a < D; ++a)
        t[a] = (_axes[a][b / stride[a] % _axes[a].size()] - _center[a]) / _half[a];
      for (std::size_t k = 0; k < T; ++k)
      {
        Real phi = 1;
        for (std::size_t i = 0; i < _active.size(); ++i)
          phi *= std::pow(t[_active[i]], _terms[k][i]);
        A(b, k) = phi;
      }
    }

    _coef.assign(T * _n, 0.);
    libMesh::DenseVector<Real> rhs(B), coef(T);
    for (unsigned int c = 0; c < _n; ++c)
    {
      for (std::size_t b = 0; b < B; ++b)
        rhs(b) = _y[b * _n + c];
      A.svd_solve(rhs, coef);
      for (std::size_t k = 0; k < T; ++k)
        _coef[k * _n + c] = coef(k);
    }

    _pow.assign(_active.size() * (_order + 1), 1.);
    _phi.resize(T);
    _dphi.resize(T);
  }

  void evaluate(const Real * x, unsigned int d_axis, Real * values, Real * dvalues) const override
  {
    // Powers of the scaled coordinate of each active axis; the derivative axis, if active and
    // inside the range
    std::size_t d = _active.size();
    for (std::size_t i = 0; i < _active.size(); ++i)
    {
      const auto a = _active[i];
      const auto & g = _axes[a];
      const Real xc = std::min(std::max(x[a], g.front()), g.back());
      if (a == d_axis && xc == x[a])
        d = i;
      const Real t = (xc - _center[a]) / _half[a];
      Real * p = &_pow[i * (_order + 1)];
      for (unsigned int k = 1; k <= _order; ++k)
        p[k] = p[k - 1] * t;
    }

    const std::size_t T = _terms.size();
    const bool derivatives = dvalues && d < _active.size();
    for (std::size_t k = 0; k < T; ++k)
    {
      Real phi = 1;
      for (std::size_t i = 0; i < _active.size(); ++i)
        phi *= _pow[i * (_order + 1) + _terms[k][i]];
      _phi[k] = phi;
      if (derivatives)
      {
        const unsigned int ed = _terms[k][d];
        Real dphi = ed == 0 ? 0. : ed * _pow[d * (_order + 1) + ed - 1] / _half[_active[d]];
        for (std::size_t i = 0; i < _active.size() && dphi != 0; ++i)
          if (i != d)
            dphi *= _pow[i * (_order + 1) + _terms[k][i]];
        _dphi[k] = dphi;
      }
    }

    std::fill(values, values + _n, 0.);
    for (std::size_t k = 0; k < T; ++k)
    {
      const Real * coef = &_coef[k * _n];
      for (unsigned int c = 0; c < _n; ++c)
        values[c] += coef[c] * _phi[k];
    }
    if (!dvalues)
      return;
    std::fill(dvalues, dvalues + _n, 0.);
    if (!derivatives)
      return;
    for (std::size_t k = 0; k < T; ++k)
    {
      const Real * coef = &_coef[k * _n];
      for (unsigned int c = 0; c < _n; ++c)
        dvalues[c] += coef[c] * _dphi[k];
    }
  }

private:
  const unsigned int _order;
  /// Axes with more than one knot
  std::vector<std::size_t> _active;
  /// Center and half-width of each axis range
  std::vector<Real> _center;
  std::vector<Real> _half;
  /// Exponent of each active axis in each term
  std::vector<std::vector<unsigned int>> _terms;
  /// Term-major coefficients
  std::vector<Real> _coef;

  /// Scratch for evaluate()
  mutable std::vector<Real> _pow;
  mutable std::vector<Real> _phi;
  mutable std::vector<Real> _dphi;
};

/// A 1-D cubic; evaluates the cubic of the end interval outside the grid
class XsCubicBasis : public XsBasis
{
public:
  void setData(const XsAxes & axes, unsigned int n, std::vector<Real> data) override
  {
    if (axes.size() != 1)
      mooseError("Spline and monotone cubic interpolation support tables with one axis only.");
    XsBasis::setData(axes, n, std::move(data));
  }

  void evaluate(const Real * x, unsigned int d_axis, Real * values, Real * dvalues) const override
  {
    evaluateCubic(x[0], values, d_axis == 0 ? dvalues : nullptr);
    if (dvalues && d_axis != 0)
      std::fill(dvalues, dvalues + _n, 0.);
  }

protected:
  /// Evaluates the cubic of the interval that contains x (the end interval outside the grid)
  virtual void evaluateCubic(Real x, Real * values, Real * dvalues) const = 0;
};

/// Natural cubic spline; extrapolates the end cubics outside the grid
class XsSplineBasis : public XsCubicBasis
{
public:

  void setData(const XsAxes & axes, unsigned int n, std::vector<Real> data) override
  {
    XsCubicBasis::setData(axes, n, std::move(data));
    if (_axes[0].size() < 2)
      mooseError("You must have at least two knots to create a spline.");

    const auto & grid = _axes[0];
    const auto L = grid.size();
    _y2.assign(L * _n, 0.);
    std::vector<Real> y(L), y2(L), u(L);
    for (unsigned int c = 0; c < _n; ++c)
    {
      for (unsigned int k = 0; k < L; ++k)
        y[k] = _y[k * _n + c];
      secondDerivatives(grid, y, y2, u);
      for (unsigned int k = 0; k < L; ++k)
        _y2[k * _n + c] = y2[k];
    }
  }

protected:
  void evaluateCubic(Real x, Real * values, Real * dvalues) const override
  {
    const auto & grid = _axes[0];
    const unsigned int klo = findInterval(x);
    const unsigned int khi = klo + 1;
    const Real h = grid[khi] - grid[klo];
    const Real a = (grid[khi] - x) / h;
    const Real b = (x - grid[klo]) / h;
    const Real a3 = a * a * a - a;
    const Real b3 = b * b * b - b;
    const Real hh = h * h;

    const Real * ylo = &_y[klo * _n];
    const Real * yhi = ylo + _n;
    const Real * y2lo = &_y2[klo * _n];
    const Real * y2hi = y2lo + _n;
    for (unsigned int c = 0; c < _n; ++c)
      values[c] = a * ylo[c] + b * yhi[c] + (a3 * y2lo[c] + b3 * y2hi[c]) * hh / 6.0;

    if (dvalues)
    {
      const Real a2 = 3.0 * a * a - 1.0;
      const Real b2 = 3.0 * b * b - 1.0;
      for (unsigned int c = 0; c < _n; ++c)
        dvalues[c] = (yhi[c] - ylo[c]) / h - ((a2 * y2lo[c] + b2 * -y2hi[c]) * h / 6.0);
    }
  }

private:
  /// Second derivatives at the knots with natural end conditions (SplineInterpolationBase::spline)
  static void secondDerivatives(const std::vector<Real> & x,
                                const std::vector<Real> & y,
                                std::vector<Real> & y2,
                                std::vector<Real> & u)
  {
    const auto n = x.size();
    std::fill(u.begin(), u.end(), 0.);
    std::fill(y2.begin(), y2.end(), 0.);
    for (std::size_t i = 1; i < n - 1; i++)
    {
      Real sig = (x[i] - x[i - 1]) / (x[i + 1] - x[i - 1]);
      Real p = sig * y2[i - 1] + 2.0;
      y2[i] = (sig - 1.0) / p;
      u[i] = (y[i + 1] - y[i]) / (x[i + 1] - x[i]) - (y[i] - y[i - 1]) / (x[i] - x[i - 1]);
      u[i] = (6.0 * u[i] / (x[i + 1] - x[i - 1]) - sig * u[i - 1]) / p;
    }
    const Real qn = 0., un = 0.;
    y2[n - 1] = (un - qn * u[n - 2]) / (qn * y2[n - 2] + 1.);
    for (auto k = n - 1; k >= 1; k--)
      y2[k - 1] = y2[k - 1] * y2[k] + u[k - 1];
  }

  /// Knot-major second derivatives
  std::vector<Real> _y2;
};

/// Monotone piecewise cubic Hermite (Fritsch-Carlson); extrapolates the end cubics
class XsMonotoneCubicBasis : public XsCubicBasis
{
public:

  void setData(const XsAxes & axes, unsigned int n, std::vector<Real> data) override
  {
    XsCubicBasis::setData(axes, n, std::move(data));
    if (_axes[0].size() < 3)
      mooseError("Monotone cubic interpolation requires at least three data points.");

    const auto & grid = _axes[0];
    const auto L = grid.size();
    _yp.assign(L * _n, 0.);
    std::vector<Real> y(L), yp(L);
    for (unsigned int c = 0; c < _n; ++c)
    {
      for (unsigned int k = 0; k < L; ++k)
        y[k] = _y[k * _n + c];
      slopes(grid, y, yp);
      for (unsigned int k = 0; k < L; ++k)
        _yp[k * _n + c] = yp[k];
    }
  }

protected:
  void evaluateCubic(Real x, Real * values, Real * dvalues) const override
  {
    const unsigned int klo = findInterval(x);
    const unsigned int khi = klo + 1;
    const Real xlo = _axes[0][klo];
    const Real xhi = _axes[0][khi];

    const Real w1 = h1(xhi, xlo, x);
    const Real w2 = h2(xhi, xlo, x);
    const Real w3 = h3(xhi, xlo, x);
    const Real w4 = h4(xhi, xlo, x);

    const Real * flo = &_y[klo * _n];
    const Real * fhi = flo + _n;
    const Real * dlo = &_yp[klo * _n];
    const Real * dhi = dlo + _n;
    for (unsigned int c = 0; c < _n; ++c)
      values[c] = flo[c] * w1 + fhi[c] * w2 + dlo[c] * w3 + dhi[c] * w4;

    if (dvalues)
    {
      const Real w1p = h1Prime(xhi, xlo, x);
      const Real w2p = h2Prime(xhi, xlo, x);
      const Real w3p = h3Prime(xhi, xlo, x);
      const Real w4p = h4Prime(xhi, xlo, x);
      for (unsigned int c = 0; c < _n; ++c)
        dvalues[c] = flo[c] * w1p + fhi[c] * w2p + dlo[c] * w3p + dhi[c] * w4p;
    }
  }

private:
  static Real sign(Real x) { return x < 0 ? -1 : (x > 0 ? 1 : 0); }
  static Real phi(Real t) { return 3. * t * t - 2. * t * t * t; }
  static Real phiPrime(Real t) { return 6. * t - 6. * t * t; }
  static Real psi(Real t) { return t * t * t - t * t; }
  static Real psiPrime(Real t) { return 3. * t * t - 2. * t; }

  static Real h1(Real xhi, Real xlo, Real x)
  {
    Real h = xhi - xlo;
    Real t = (xhi - x) / h;
    return phi(t);
  }
  static Real h2(Real xhi, Real xlo, Real x)
  {
    Real h = xhi - xlo;
    Real t = (x - xlo) / h;
    return phi(t);
  }
  static Real h3(Real xhi, Real xlo, Real x)
  {
    Real h = xhi - xlo;
    Real t = (xhi - x) / h;
    return -h * psi(t);
  }
  static Real h4(Real xhi, Real xlo, Real x)
  {
    Real h = xhi - xlo;
    Real t = (x - xlo) / h;
    return h * psi(t);
  }
  static Real h1Prime(Real xhi, Real xlo, Real x)
  {
    Real h = xhi - xlo;
    Real t = (xhi - x) / h;
    Real tPrime = -1. / h;
    return phiPrime(t) * tPrime;
  }
  static Real h2Prime(Real xhi, Real xlo, Real x)
  {
    Real h = xhi - xlo;
    Real t = (x - xlo) / h;
    Real tPrime = 1. / h;
    return phiPrime(t) * tPrime;
  }
  static Real h3Prime(Real xhi, Real xlo, Real x)
  {
    Real h = xhi - xlo;
    Real t = (xhi - x) / h;
    Real tPrime = -1. / h;
    return -h * psiPrime(t) * tPrime;
  }
  static Real h4Prime(Real xhi, Real xlo, Real x)
  {
    Real h = xhi - xlo;
    Real t = (x - xlo) / h;
    Real tPrime = 1. / h;
    return h * psiPrime(t) * tPrime;
  }

  /// Knot slopes (MonotoneCubicInterpolation::solve)
  static void slopes(const std::vector<Real> & x, const std::vector<Real> & y, std::vector<Real> & yp)
  {
    const unsigned int n_knots = x.size();
    const unsigned int n_intervals = n_knots - 1;
    std::vector<Real> h(n_intervals), delta(n_intervals);
    for (unsigned int i = 0; i < n_intervals; ++i)
      h[i] = x[i + 1] - x[i];

    // Three-point parabolic estimates
    for (unsigned int i = 1; i < n_knots - 1; ++i)
      yp[i] = (std::pow(h[i - 1], 2) * y[i + 1] - std::pow(h[i], 2) * y[i - 1] -
               y[i] * (h[i - 1] - h[i]) * (h[i - 1] + h[i])) /
              (h[i - 1] * h[i] * (h[i - 1] * h[i]));
    yp[0] = (-std::pow(h[0], 2) * y[2] - h[1] * y[0] * (2 * h[0] + h[1]) +
             y[1] * std::pow(h[0] + h[1], 2)) /
            (h[0] * h[1] * (h[0] + h[1]));
    Real hlast = h[n_intervals - 1];
    Real hsecond = h[n_intervals - 2];
    Real ylast = y[n_knots - 1];
    Real ysecond = y[n_knots - 2];
    Real ythird = y[n_knots - 3];
    yp[n_knots - 1] = (hsecond * ylast * (hsecond + 2 * hlast) + std::pow(hlast, 2) * ythird -
                       ysecond * std::pow(hsecond + hlast, 2)) /
                      (hsecond * hlast * (hsecond + hlast));

    for (unsigned int i = 0; i < n_intervals; ++i)
      delta[i] = (y[i + 1] - y[i]) / h[i];
    if (sign(delta[0]) != sign(yp[0]))
      yp[0] = 0;
    if (sign(delta[n_intervals - 1]) != sign(yp[n_knots - 1]))
      yp[n_knots - 1] = 0;
    for (unsigned int i = 0; i < n_knots - 2; ++i)
      if (sign(delta[i + 1]) == 0 || sign(delta[i]) == 0 || sign(delta[i + 1]) != sign(delta[i]))
        yp[1 + i] = 0;

    // Limit the slopes to the Fritsch-Carlson monotonicity region, interval by interval
    for (unsigned int i = 0; i < n_intervals; ++i)
    {
      Real alpha, beta;
      if (yp[i] == 0)
        alpha = 0;
      else if (delta[i] == 0)
        alpha = 4;
      else
        alpha = yp[i] / delta[i];

      if (yp[i + 1] == 0)
        beta = 0;
      else if (delta[i] == 0)
        beta = 4;
      else
        beta = yp[i + 1] / delta[i];

      if (std::pow(alpha, 2) + std::pow(beta, 2) > 9.)
      {
        Real tau = 3. / std::sqrt(std::pow(alpha, 2) + std::pow(beta, 2));
        yp[i] = alpha * tau * delta[i];
        yp[i + 1] = beta * tau * delta[i];
      }
    }
  }

  /// Knot-major slopes
  std::vector<Real> _yp;
};

} // namespace

void
XsBasis::setData(const XsAxes & axes, unsigned int n, std::vector<Real> data)
{
  for (const auto & grid : axes)
    for (std::size_t k = 0; k + 1 < grid.size(); ++k)
      if (grid[k] >= grid[k + 1])
        mooseError("x-values are not strictly increasing");
  _axes = axes;
  _n = n;
  _y = std::move(data);
}

unsigned int
XsBasis::findInterval(Real x) const
{
  const auto & grid = _axes[0];
  unsigned int klo = 0;
  unsigned int khi = grid.size() - 1;
  while (khi - klo > 1)
  {
    unsigned int k = (khi + klo) >> 1;
    if (grid[k] > x)
      khi = k;
    else
      klo = k;
  }
  return klo;
}

std::unique_ptr<XsBasis>
XsBasis::build(XsInterpolation type, unsigned int order)
{
  switch (type)
  {
    case XsInterpolation::NONE:
      return std::make_unique<XsNoneBasis>();
    case XsInterpolation::LINEAR:
      return std::make_unique<XsMultilinearBasis>();
    case XsInterpolation::SPLINE:
      return std::make_unique<XsSplineBasis>();
    case XsInterpolation::MONOTONE_CUBIC:
      return std::make_unique<XsMonotoneCubicBasis>();
    case XsInterpolation::LEAST_SQUARES:
      return std::make_unique<XsLeastSquaresBasis>(order);
  }
  mooseError("Unknown interpolation type");
}

unsigned int
XsTable::addComponents(const XsAxes & axes,
                       const std::vector<std::vector<Real>> & data,
                       const std::string & name)
{
  const unsigned int offset = _values.size();
  if (data.empty())
    return offset;

  if (_lower.empty())
  {
    _lower.assign(axes.size(), -std::numeric_limits<Real>::infinity());
    _upper.assign(axes.size(), std::numeric_limits<Real>::infinity());
    _sampled_min.assign(axes.size(), std::numeric_limits<Real>::infinity());
    _sampled_max.assign(axes.size(), -std::numeric_limits<Real>::infinity());
  }
  else if (axes.size() != _lower.size())
    mooseError("The ", name, " table has ", axes.size(), " axes, other group constants have ",
               _lower.size(), ".");

  std::size_t branches = 1;
  for (std::size_t a = 0; a < axes.size(); ++a)
  {
    if (axes[a].empty())
      mooseError("Axis ", a, " of the ", name, " table has no knots.");
    branches *= axes[a].size();
    _lower[a] = std::max(_lower[a], axes[a].front());
    _upper[a] = std::min(_upper[a], axes[a].back());
  }
  for (const auto & component : data)
    if (component.size() != branches)
      mooseError("The ", name, " table has ", component.size(), " values for ", branches,
                 " grid points.");

  if (_blocks.empty() || _blocks.back().axes != axes)
    _blocks.push_back({offset, axes, {}, nullptr});
  auto & block = _blocks.back();
  block.data.insert(block.data.end(), data.begin(), data.end());

  _names.push_back({name, offset, static_cast<unsigned int>(data.size())});
  _values.resize(offset + data.size());
  _dvalues.resize(offset + data.size());
  return offset;
}

void
XsTable::finalize()
{
  // Largest deviation from the data at the knots of each component, relative to its scale
  std::vector<Real> residual(_values.size(), 0.);
  for (auto & block : _blocks)
  {
    const unsigned int n = block.data.size();
    const auto B = block.data[0].size();
    std::vector<Real> branch_major(B * n);
    for (unsigned int c = 0; c < n; ++c)
      for (std::size_t b = 0; b < B; ++b)
        branch_major[b * n + c] = block.data[c][b];

    block.basis = XsBasis::build(_type, _order);
    block.basis->setData(block.axes, n, branch_major);

    if (_type == XsInterpolation::LEAST_SQUARES)
    {
      const auto D = block.axes.size();
      std::vector<Real> x(D), fit(n), scale(n, 0.);
      for (unsigned int c = 0; c < n; ++c)
        for (std::size_t b = 0; b < B; ++b)
          scale[c] = std::max(scale[c], std::abs(block.data[c][b]));
      for (std::size_t b = 0; b < B; ++b)
      {
        for (std::size_t a = D, r = b; a-- > 0; r /= block.axes[a].size())
          x[a] = block.axes[a][r % block.axes[a].size()];
        block.basis->evaluate(x.data(), D, fit.data(), nullptr);
        for (unsigned int c = 0; c < n; ++c)
          if (scale[c] > 0)
            residual[block.offset + c] = std::max(residual[block.offset + c],
                                                  std::abs(fit[c] - block.data[c][b]) / scale[c]);
      }
    }
    block.data.clear();
  }

  _residuals.clear();
  for (const auto & named : _names)
    _residuals.emplace_back(named.name,
                            named.size ? *std::max_element(residual.begin() + named.offset,
                                                           residual.begin() + named.offset +
                                                               named.size)
                                       : 0.);
  _cache_valid = false;
}

void
XsTable::evaluate(const std::vector<Real> & x, unsigned int d_axis, bool derivatives)
{
  if (_cache_valid && x == _cache_x &&
      (!derivatives || (_cache_has_derivatives && d_axis == _cache_d_axis)))
    return;

  for (std::size_t a = 0; a < x.size(); ++a)
  {
    _sampled_min[a] = std::min(_sampled_min[a], x[a]);
    _sampled_max[a] = std::max(_sampled_max[a], x[a]);
  }

  for (const auto & block : _blocks)
    block.basis->evaluate(
        x.data(), d_axis, &_values[block.offset], derivatives ? &_dvalues[block.offset] : nullptr);

  _cache_valid = true;
  _cache_x = x;
  _cache_has_derivatives = derivatives;
  _cache_d_axis = d_axis;
}
