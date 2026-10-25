#pragma once

#include "SpatialUserObjectFunctor.h"
#include "UserObject.h"
#include "MooseEnum.h"
#include "MooseVariableFieldBase.h"
#include "libmesh/dense_matrix.h"
#include "libmesh/dense_vector.h"
#include "libmesh/fe_lagrange_shape_1D.h"

#include <algorithm>
#include <array>

/**
 * L2 projection of a variable u onto the fields that vary only along one coordinate direction z
 * and are continuous and piecewise polynomial (C0 Pp) of order p (1 to 3) in z. The layers are
 * the elements of that 1-D field, so the layer bounds must be the element boundaries of the
 * receiving 1-D mesh (e.g. a SAM flow channel).
 *
 * With phi_i the Lagrange basis functions of order p of the 1-D field (p N + 1 nodes for N
 * layers, equally spaced within each layer), the object accumulates over the blocks (volume
 * variant) or boundaries (side variant) it acts on
 *
 *   b_i  = int u phi_i dV,      M_ij = int phi_i phi_j dV,
 *
 * and solves M x = b. This is a single projection; the reduction to 1-D follows from the target
 * fields being constant across the direction. Integrating over the cross-section first gives the
 * equivalent form
 *
 *   b_i  = int A(z) ubar(z) phi_i dz,      M_ij = int A(z) phi_i phi_j dz,
 *
 * the projection of the cross-section average ubar(z) weighted by the cross-section area (or
 * boundary width) A(z). With a piecewise-constant, discontinuous basis instead (p = 0), it would
 * reduce to a layered average.
 *
 * x holds the nodal values of the projected field, and spatialValue(p) returns its interpolant
 * of order p at p, so a nodal LAGRANGE target variable of the same order receives the projection
 * exactly through MultiAppGeneralFieldUserObjectTransfer. Each quadrature point is assigned to
 * the layer containing it, so the source mesh need not be aligned with the layers, but it must
 * resolve every layer along the direction (at least p + 1 distinct quadrature point positions
 * per layer); finalize() checks that every layer's mass block is nonsingular.
 *
 * Compared with a layered average, the projection is accurate to order p + 1 in the layer size
 * for smooth fields (instead of first order) and conserves the integral of u exactly (as
 * evaluated with the source quadrature; the basis functions sum to one), but it can
 * over/undershoot if u has a discontinuity inside a layer. The source variable must not be of
 * CONSTANT order, since it then carries no information below the source element size (use a
 * layered average instead); its order should be at least p.
 */
template <typename UserObjectType>
class DirectionalL2ProjectionBase : public SpatialUserObjectFunctor<UserObjectType>
{
public:
  static InputParameters validParams();

  DirectionalL2ProjectionBase(const InputParameters & parameters);

  virtual void initialize() override;
  virtual void execute() override;
  virtual void threadJoin(const UserObject & y) override;
  virtual void finalize() override;

  /// Interpolant of the projected field at p (constant beyond the end layers)
  virtual Real spatialValue(const Point & p) const override;

  /// Positions of the p N + 1 nodes of the projected field along the direction
  virtual const std::vector<Point> spatialPoints() const override;

  /// Nodal values of the projected field, ordered along the direction
  const std::vector<Real> & nodalValues() const { return _nodal_values; }

  /// Direction of the layers (0, 1, 2 for x, y, z)
  unsigned int direction() const { return _direction; }

  /// Highest supported order of the projected field
  static constexpr unsigned int max_order = 3;

protected:
  using UserObjectType::_coord;
  using UserObjectType::_JxW;
  using UserObjectType::_q_point;
  using UserObjectType::_qrule;

  /// Lagrange basis of order _order on [0, 1] at the layer's nodes m / _order, left to right
  void basis(Real xi, std::array<Real, max_order + 1> & phi) const;

  /// Layer containing coordinate z (end layers for points on or beyond the outer bounds)
  unsigned int layerIndex(Real z) const;

  /// Index of the (i, j) entry (i <= j) of a layer's packed symmetric mass block
  unsigned int massIndex(unsigned int i, unsigned int j) const;

  /// Variable being projected
  const VariableValue & _u;

  /// Direction of the layers
  const unsigned int _direction;

  /// Order p of the projected field
  const unsigned int _order;

  /// Number of nodes per layer (p + 1)
  const unsigned int _n_local;

  /// Number of entries of a layer's packed symmetric mass block ((p + 1) (p + 2) / 2)
  const unsigned int _n_mass;

  /// Layer bounds along the direction (N + 1 values)
  std::vector<Real> _bounds;

  /// Number of layers
  unsigned int _num_layers;

  /// Allowed distance of quadrature points beyond the outer bounds
  Real _tolerance;

  /// Load vector b (p N + 1)
  std::vector<Real> _load;

  /// Per-layer packed symmetric mass blocks, _n_mass entries per layer
  std::vector<Real> _mass;

  /// Projected nodal values (p N + 1)
  std::vector<Real> _nodal_values;
};

template <typename UserObjectType>
InputParameters
DirectionalL2ProjectionBase<UserObjectType>::validParams()
{
  InputParameters params = SpatialUserObjectFunctor<UserObjectType>::validParams();
  params.addRequiredCoupledVar("variable", "The variable to project onto the 1-D field");
  MooseEnum directions("x y z");
  params.addRequiredParam<MooseEnum>("direction", directions, "The direction of the layers");
  params.addParam<std::vector<Real>>(
      "bounds",
      "Layer bounds along the direction, i.e. the element boundaries of the receiving 1-D "
      "mesh, in increasing order. Alternative to num_layers/direction_min/direction_max.");
  params.addParam<unsigned int>("num_layers", "Number of uniform layers");
  params.addParam<Real>("direction_min", "Lower bound of the uniform layers");
  params.addParam<Real>("direction_max", "Upper bound of the uniform layers");
  MooseEnum orders("FIRST=1 SECOND=2 THIRD=3");
  params.addRequiredParam<MooseEnum>(
      "order",
      orders,
      "Order of the projected field; it should match the order of the LAGRANGE variable that "
      "receives the projection");
  params.addRangeCheckedParam<Real>(
      "bounds_tolerance",
      1e-8,
      "bounds_tolerance >= 0",
      "Relative tolerance (fraction of the total layer extent) for quadrature points lying "
      "beyond the outer bounds; points farther out are an error");
  return params;
}

template <typename UserObjectType>
DirectionalL2ProjectionBase<UserObjectType>::DirectionalL2ProjectionBase(
    const InputParameters & parameters)
  : SpatialUserObjectFunctor<UserObjectType>(parameters),
    _u(this->coupledValue("variable")),
    _direction(this->template getParam<MooseEnum>("direction")),
    _order(this->template getParam<MooseEnum>("order")),
    _n_local(_order + 1),
    _n_mass(_n_local * (_n_local + 1) / 2)
{
  const bool have_bounds = this->isParamValid("bounds");
  const bool have_uniform = this->isParamValid("num_layers") ||
                            this->isParamValid("direction_min") ||
                            this->isParamValid("direction_max");
  if (have_bounds == have_uniform)
    this->paramError("bounds",
                     "Specify either 'bounds' or all of 'num_layers', 'direction_min' and "
                     "'direction_max'");

  if (have_bounds)
    _bounds = this->template getParam<std::vector<Real>>("bounds");
  else
  {
    if (!this->isParamValid("num_layers") || !this->isParamValid("direction_min") ||
        !this->isParamValid("direction_max"))
      this->paramError("num_layers",
                       "'num_layers', 'direction_min' and 'direction_max' must all be given");
    const auto n = this->template getParam<unsigned int>("num_layers");
    const auto zmin = this->template getParam<Real>("direction_min");
    const auto zmax = this->template getParam<Real>("direction_max");
    if (n == 0)
      this->paramError("num_layers", "Must be positive");
    if (!(zmax > zmin))
      this->paramError("direction_max", "Must be larger than 'direction_min'");
    _bounds.resize(n + 1);
    for (unsigned int k = 0; k <= n; ++k)
      _bounds[k] = zmin + (zmax - zmin) * k / n;
  }

  if (_bounds.size() < 2)
    this->paramError("bounds", "At least two bounds (one layer) are required");
  for (const auto k : make_range(_bounds.size() - 1))
    if (!(_bounds[k + 1] > _bounds[k]))
      this->paramError("bounds", "Bounds must be strictly increasing");

  _num_layers = _bounds.size() - 1;
  _tolerance =
      this->template getParam<Real>("bounds_tolerance") * (_bounds.back() - _bounds.front());

  _load.assign(_order * _num_layers + 1, 0.);
  _mass.assign(_n_mass * _num_layers, 0.);
  _nodal_values.assign(_order * _num_layers + 1, 0.);

  if (this->getFieldVar("variable", 0)->feType().order == CONSTANT)
    this->paramError("variable",
                     "The projected variable is of CONSTANT order, so it carries no information "
                     "below its element size; use a layered average for it instead");
}

template <typename UserObjectType>
void
DirectionalL2ProjectionBase<UserObjectType>::basis(Real xi,
                                                   std::array<Real, max_order + 1> & phi) const
{
  // libMesh's reference edge is [-1, 1] and numbers the end nodes first, then the interior nodes
  const Real eta = 2. * xi - 1.;
  for (unsigned int m = 0; m < _n_local; ++m)
  {
    const unsigned int i = m == 0 ? 0 : (m == _order ? 1 : m + 1);
    phi[m] = fe_lagrange_1D_shape(static_cast<Order>(_order), i, eta);
  }
}

template <typename UserObjectType>
unsigned int
DirectionalL2ProjectionBase<UserObjectType>::layerIndex(Real z) const
{
  const auto it = std::upper_bound(_bounds.begin(), _bounds.end(), z);
  if (it == _bounds.begin())
    return 0;
  return std::min(static_cast<unsigned int>(std::distance(_bounds.begin(), it) - 1),
                  _num_layers - 1);
}

template <typename UserObjectType>
unsigned int
DirectionalL2ProjectionBase<UserObjectType>::massIndex(unsigned int i, unsigned int j) const
{
  // Row-major upper triangle: (0,0) (0,1) ... (0,p) (1,1) ... (p,p)
  return i * (2 * _n_local - i + 1) / 2 + (j - i);
}

template <typename UserObjectType>
void
DirectionalL2ProjectionBase<UserObjectType>::initialize()
{
  // (UserObjectBase::initialize is pure virtual; Element/SideUserObject have nothing to reset)
  std::fill(_load.begin(), _load.end(), 0.);
  std::fill(_mass.begin(), _mass.end(), 0.);
}

template <typename UserObjectType>
void
DirectionalL2ProjectionBase<UserObjectType>::execute()
{
  for (unsigned int qp = 0; qp < _qrule->n_points(); ++qp)
  {
    const Real z = _q_point[qp](_direction);
    if (z < _bounds.front() - _tolerance || z > _bounds.back() + _tolerance)
      mooseError(this->name(),
                 ": quadrature point at ",
                 z,
                 " lies outside the layer bounds [",
                 _bounds.front(),
                 ", ",
                 _bounds.back(),
                 "] along the projection direction");

    const auto k = layerIndex(z);
    const Real xi = (z - _bounds[k]) / (_bounds[k + 1] - _bounds[k]);
    std::array<Real, max_order + 1> phi;
    basis(xi, phi);

    const Real w = _JxW[qp] * _coord[qp];
    for (unsigned int i = 0; i < _n_local; ++i)
    {
      _load[_order * k + i] += _u[qp] * phi[i] * w;
      for (unsigned int j = i; j < _n_local; ++j)
        _mass[_n_mass * k + massIndex(i, j)] += phi[i] * phi[j] * w;
    }
  }
}

template <typename UserObjectType>
void
DirectionalL2ProjectionBase<UserObjectType>::threadJoin(const UserObject & y)
{
  // (UserObject::threadJoin is pure virtual; Element/SideUserObject add nothing to join)
  const auto & other = static_cast<const DirectionalL2ProjectionBase<UserObjectType> &>(y);
  for (const auto i : index_range(_load))
    _load[i] += other._load[i];
  for (const auto i : index_range(_mass))
    _mass[i] += other._mass[i];
}

template <typename UserObjectType>
void
DirectionalL2ProjectionBase<UserObjectType>::finalize()
{
  this->_communicator.sum(_load);
  this->_communicator.sum(_mass);

  // Each layer's mass block must be nonsingular, i.e. the source mesh must resolve the layer
  // along the direction. Normalized determinant det / prod(m_ii): 0.75, 0.78 and 0.71 for a
  // well-resolved layer of order 1, 2 and 3, and 0 for a layer seen at fewer than p + 1
  // distinct positions.
  for (const auto k : make_range(_num_layers))
  {
    DenseMatrix<Real> block(_n_local, _n_local);
    Real diag = 1.;
    for (unsigned int i = 0; i < _n_local; ++i)
    {
      for (unsigned int j = i; j < _n_local; ++j)
        block(i, j) = block(j, i) = _mass[_n_mass * k + massIndex(i, j)];
      diag *= block(i, i);
    }
    // The block is symmetric positive semidefinite, so its determinant is the product of its
    // singular values (DenseMatrix::det() would throw on an exactly singular block)
    Real ratio = 0.;
    if (diag > 0.)
    {
      DenseVector<Real> sigma;
      block.svd(sigma);
      ratio = 1. / diag;
      for (const auto i : make_range(sigma.size()))
        ratio *= sigma(i);
    }

    if (ratio < 1e-6)
      mooseError(this->name(),
                 ": layer ",
                 k,
                 " [",
                 _bounds[k],
                 ", ",
                 _bounds[k + 1],
                 "] is not resolved by the source mesh (normalized mass determinant ",
                 ratio,
                 "). The source mesh must be at least as fine as the layers along the "
                 "direction, with at least ",
                 _n_local,
                 " distinct quadrature point positions per layer.");
  }

  const auto n = _order * _num_layers + 1;
  DenseMatrix<Real> mass(n, n);
  DenseVector<Real> load(n);
  for (const auto k : make_range(_num_layers))
    for (unsigned int i = 0; i < _n_local; ++i)
    {
      load(_order * k + i) = _load[_order * k + i];
      for (unsigned int j = 0; j < _n_local; ++j)
        mass(_order * k + i, _order * k + j) +=
            _mass[_n_mass * k + massIndex(std::min(i, j), std::max(i, j))];
    }

  DenseVector<Real> x(n);
  mass.cholesky_solve(load, x);
  for (const auto i : make_range(n))
    _nodal_values[i] = x(i);
}

template <typename UserObjectType>
Real
DirectionalL2ProjectionBase<UserObjectType>::spatialValue(const Point & p) const
{
  const Real z = p(_direction);
  const auto k = layerIndex(z);
  const Real xi = std::clamp((z - _bounds[k]) / (_bounds[k + 1] - _bounds[k]), Real(0.), Real(1.));
  std::array<Real, max_order + 1> phi;
  basis(xi, phi);
  Real value = 0.;
  for (unsigned int m = 0; m < _n_local; ++m)
    value += _nodal_values[_order * k + m] * phi[m];
  return value;
}

template <typename UserObjectType>
const std::vector<Point>
DirectionalL2ProjectionBase<UserObjectType>::spatialPoints() const
{
  std::vector<Point> points(_order * _num_layers + 1);
  for (const auto k : make_range(_num_layers))
    for (unsigned int m = 0; m < _order; ++m)
      points[_order * k + m](_direction) =
          _bounds[k] + (_bounds[k + 1] - _bounds[k]) * Real(m) / _order;
  points.back()(_direction) = _bounds.back();
  return points;
}
