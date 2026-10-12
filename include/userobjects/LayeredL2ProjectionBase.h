#pragma once

#include "SpatialUserObjectFunctor.h"
#include "UserObject.h"
#include "MooseEnum.h"
#include "MooseVariableFieldBase.h"
#include "libmesh/dense_matrix.h"
#include "libmesh/dense_vector.h"

#include <algorithm>

/**
 * Weighted L2 projection of a variable onto a continuous, piecewise-quadratic (C0 P2) field
 * along one coordinate direction. The layers are the elements of that 1-D field, so the layer
 * bounds must be the element boundaries of the receiving 1-D mesh (e.g. a SAM flow channel).
 *
 * With phi_i the quadratic Lagrange basis functions of the 1-D field (2 N + 1 nodes for N
 * layers), the object accumulates over the blocks (volume variant) or boundaries (side variant)
 * it acts on
 *
 *   b_i  = int u phi_i dV,      M_ij = int phi_i phi_j dV,
 *
 * and solves M x = b. x holds the nodal values of the projected field, and spatialValue(p)
 * returns its quadratic interpolant at p, so a nodal LAGRANGE SECOND target variable receives
 * the projection exactly through MultiAppGeneralFieldUserObjectTransfer. Each quadrature point
 * is assigned to the layer containing it, so the source mesh need not be aligned with the
 * layers, but it must resolve every layer along the direction (at least 3 distinct quadrature
 * point positions per layer); finalize() checks that every layer's 3x3 mass block is
 * nonsingular.
 *
 * Compared with a layered average, the projection is third-order accurate in the layer size
 * for smooth fields (instead of first order) and conserves the integral of u exactly, but it
 * can over/undershoot if u has a discontinuity inside a layer. The source variable should be
 * of second order (e.g. L2_LAGRANGE SECOND or LAGRANGE SECOND); a CONSTANT source carries no
 * information below the source element size.
 */
template <typename UserObjectType>
class LayeredL2ProjectionBase : public SpatialUserObjectFunctor<UserObjectType>
{
public:
  static InputParameters validParams();

  LayeredL2ProjectionBase(const InputParameters & parameters);

  virtual void initialize() override;
  virtual void execute() override;
  virtual void threadJoin(const UserObject & y) override;
  virtual void finalize() override;

  /// Quadratic interpolant of the projected field at p (constant beyond the end layers)
  virtual Real spatialValue(const Point & p) const override;

  /// Positions of the 2 N + 1 nodes of the projected field along the direction
  virtual const std::vector<Point> spatialPoints() const override;

  /// Nodal values of the projected field, ordered along the direction
  const std::vector<Real> & nodalValues() const { return _nodal_values; }

  /// Direction of the layers (0, 1, 2 for x, y, z)
  unsigned int direction() const { return _direction; }

protected:
  using UserObjectType::_coord;
  using UserObjectType::_JxW;
  using UserObjectType::_q_point;
  using UserObjectType::_qrule;

  /// Quadratic Lagrange basis on [0, 1], nodes at 0, 1/2, 1
  static void basis(Real xi, Real phi[3]);

  /// Layer containing coordinate z (end layers for points on or beyond the outer bounds)
  unsigned int layerIndex(Real z) const;

  /// Index of the (i, j) entry (i <= j) of a layer's symmetric 3x3 mass block
  static unsigned int massIndex(unsigned int i, unsigned int j);

  /// Variable being projected
  const VariableValue & _u;

  /// Direction of the layers
  const unsigned int _direction;

  /// Layer bounds along the direction (N + 1 values)
  std::vector<Real> _bounds;

  /// Number of layers
  unsigned int _num_layers;

  /// Allowed distance of quadrature points beyond the outer bounds
  Real _tolerance;

  /// Load vector b (2 N + 1)
  std::vector<Real> _load;

  /// Per-layer symmetric 3x3 mass blocks, 6 entries per layer
  std::vector<Real> _mass;

  /// Projected nodal values (2 N + 1)
  std::vector<Real> _nodal_values;
};

template <typename UserObjectType>
InputParameters
LayeredL2ProjectionBase<UserObjectType>::validParams()
{
  InputParameters params = SpatialUserObjectFunctor<UserObjectType>::validParams();
  params.addRequiredCoupledVar("variable", "The variable to project onto the layered 1-D field");
  MooseEnum directions("x y z");
  params.addRequiredParam<MooseEnum>("direction", directions, "The direction of the layers");
  params.addParam<std::vector<Real>>(
      "bounds",
      "Layer bounds along the direction, i.e. the element boundaries of the receiving 1-D "
      "mesh, in increasing order. Alternative to num_layers/direction_min/direction_max.");
  params.addParam<unsigned int>("num_layers", "Number of uniform layers");
  params.addParam<Real>("direction_min", "Lower bound of the uniform layers");
  params.addParam<Real>("direction_max", "Upper bound of the uniform layers");
  params.addRangeCheckedParam<Real>(
      "bounds_tolerance",
      1e-8,
      "bounds_tolerance >= 0",
      "Relative tolerance (fraction of the total layer extent) for quadrature points lying "
      "beyond the outer bounds; points farther out are an error");
  return params;
}

template <typename UserObjectType>
LayeredL2ProjectionBase<UserObjectType>::LayeredL2ProjectionBase(
    const InputParameters & parameters)
  : SpatialUserObjectFunctor<UserObjectType>(parameters),
    _u(this->coupledValue("variable")),
    _direction(this->template getParam<MooseEnum>("direction"))
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

  _load.assign(2 * _num_layers + 1, 0.);
  _mass.assign(6 * _num_layers, 0.);
  _nodal_values.assign(2 * _num_layers + 1, 0.);

  if (this->getFieldVar("variable", 0)->feType().order == CONSTANT)
    this->paramWarning("variable",
                       "The projected variable is of CONSTANT order, so it carries no "
                       "information below its element size; use a second-order variable "
                       "(e.g. L2_LAGRANGE SECOND) for a higher-order projection");
}

template <typename UserObjectType>
void
LayeredL2ProjectionBase<UserObjectType>::basis(Real xi, Real phi[3])
{
  phi[0] = 2. * (xi - 0.5) * (xi - 1.);
  phi[1] = -4. * xi * (xi - 1.);
  phi[2] = 2. * xi * (xi - 0.5);
}

template <typename UserObjectType>
unsigned int
LayeredL2ProjectionBase<UserObjectType>::layerIndex(Real z) const
{
  const auto it = std::upper_bound(_bounds.begin(), _bounds.end(), z);
  if (it == _bounds.begin())
    return 0;
  return std::min(static_cast<unsigned int>(std::distance(_bounds.begin(), it) - 1),
                  _num_layers - 1);
}

template <typename UserObjectType>
unsigned int
LayeredL2ProjectionBase<UserObjectType>::massIndex(unsigned int i, unsigned int j)
{
  // (0,0) (0,1) (0,2) (1,1) (1,2) (2,2)
  static const unsigned int index[3][3] = {{0, 1, 2}, {1, 3, 4}, {2, 4, 5}};
  return index[i][j];
}

template <typename UserObjectType>
void
LayeredL2ProjectionBase<UserObjectType>::initialize()
{
  // (UserObjectBase::initialize is pure virtual; Element/SideUserObject have nothing to reset)
  std::fill(_load.begin(), _load.end(), 0.);
  std::fill(_mass.begin(), _mass.end(), 0.);
}

template <typename UserObjectType>
void
LayeredL2ProjectionBase<UserObjectType>::execute()
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
    Real phi[3];
    basis(xi, phi);

    const Real w = _JxW[qp] * _coord[qp];
    for (unsigned int i = 0; i < 3; ++i)
    {
      _load[2 * k + i] += _u[qp] * phi[i] * w;
      for (unsigned int j = i; j < 3; ++j)
        _mass[6 * k + massIndex(i, j)] += phi[i] * phi[j] * w;
    }
  }
}

template <typename UserObjectType>
void
LayeredL2ProjectionBase<UserObjectType>::threadJoin(const UserObject & y)
{
  // (UserObject::threadJoin is pure virtual; Element/SideUserObject add nothing to join)
  const auto & other = static_cast<const LayeredL2ProjectionBase<UserObjectType> &>(y);
  for (const auto i : index_range(_load))
    _load[i] += other._load[i];
  for (const auto i : index_range(_mass))
    _mass[i] += other._mass[i];
}

template <typename UserObjectType>
void
LayeredL2ProjectionBase<UserObjectType>::finalize()
{
  this->_communicator.sum(_load);
  this->_communicator.sum(_mass);

  // Each layer's 3x3 mass block must be nonsingular, i.e. the source mesh must resolve the
  // layer along the direction. Normalized determinant det / (m00 m11 m22): about 0.78 for a
  // well-resolved layer, 0 for a layer seen at fewer than 3 distinct positions.
  for (const auto k : make_range(_num_layers))
  {
    const Real * m = &_mass[6 * k];
    const Real diag = m[0] * m[3] * m[5];
    const Real det = m[0] * (m[3] * m[5] - m[4] * m[4]) - m[1] * (m[1] * m[5] - m[4] * m[2]) +
                     m[2] * (m[1] * m[4] - m[3] * m[2]);
    if (!(diag > 0.) || det / diag < 1e-6)
      mooseError(this->name(),
                 ": layer ",
                 k,
                 " [",
                 _bounds[k],
                 ", ",
                 _bounds[k + 1],
                 "] is not resolved by the source mesh (normalized mass determinant ",
                 diag > 0. ? det / diag : 0.,
                 "). The source mesh must be at least as fine as the layers along the "
                 "direction, with at least 3 distinct quadrature point positions per layer.");
  }

  const auto n = 2 * _num_layers + 1;
  DenseMatrix<Real> mass(n, n);
  DenseVector<Real> load(n);
  for (const auto k : make_range(_num_layers))
    for (unsigned int i = 0; i < 3; ++i)
    {
      load(2 * k + i) = _load[2 * k + i];
      for (unsigned int j = 0; j < 3; ++j)
        mass(2 * k + i, 2 * k + j) += _mass[6 * k + massIndex(std::min(i, j), std::max(i, j))];
    }

  DenseVector<Real> x(n);
  mass.cholesky_solve(load, x);
  for (const auto i : make_range(n))
    _nodal_values[i] = x(i);
}

template <typename UserObjectType>
Real
LayeredL2ProjectionBase<UserObjectType>::spatialValue(const Point & p) const
{
  const Real z = p(_direction);
  const auto k = layerIndex(z);
  const Real xi =
      std::clamp((z - _bounds[k]) / (_bounds[k + 1] - _bounds[k]), Real(0.), Real(1.));
  Real phi[3];
  basis(xi, phi);
  return _nodal_values[2 * k] * phi[0] + _nodal_values[2 * k + 1] * phi[1] +
         _nodal_values[2 * k + 2] * phi[2];
}

template <typename UserObjectType>
const std::vector<Point>
LayeredL2ProjectionBase<UserObjectType>::spatialPoints() const
{
  std::vector<Point> points(2 * _num_layers + 1);
  for (const auto k : make_range(_num_layers))
  {
    points[2 * k](_direction) = _bounds[k];
    points[2 * k + 1](_direction) = 0.5 * (_bounds[k] + _bounds[k + 1]);
  }
  points.back()(_direction) = _bounds.back();
  return points;
}
