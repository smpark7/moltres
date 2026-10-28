#pragma once

#include "MooseTypes.h"

#include <memory>
#include <string>
#include <vector>

/// Rule used to interpolate a group-constant table between its knots
enum class XsInterpolation
{
  NONE,
  LINEAR,
  SPLINE,
  MONOTONE_CUBIC,
  /// Least-squares polynomial fit of the tabulated values
  LEAST_SQUARES
};

/// Knots of each axis of a table; the state point has one coordinate per axis
using XsAxes = std::vector<std::vector<Real>>;

/**
 * Interpolates several group constants that are tabulated on the same tensor-product grid.
 * Implementations locate the cell and compute the interpolation weights once per state point
 * and apply them to every component.
 */
class XsBasis
{
public:
  virtual ~XsBasis() = default;

  /**
   * Sets the tabulated data.
   * @param axes Strictly increasing knots of each axis
   * @param n Number of components
   * @param data Branch-major values: data[b * n + c] is component c at branch b, where the branch
   *             index is row-major over the axes (the last axis varies fastest)
   */
  virtual void setData(const XsAxes & axes, unsigned int n, std::vector<Real> data);

  /**
   * Writes the n component values at the state point x (one coordinate per axis), and their
   * derivatives along axis d_axis if dvalues is not null. The derivatives are zero if d_axis is
   * not an axis of the table.
   */
  virtual void evaluate(const Real * x, unsigned int d_axis, Real * values, Real * dvalues) const = 0;

  /// Builds the basis for an interpolation rule; order is the degree of least-squares fits
  static std::unique_ptr<XsBasis> build(XsInterpolation type, unsigned int order);

protected:
  /// Index klo of the interval [x_klo, x_klo+1] of the first axis used for x, found by bisection
  unsigned int findInterval(Real x) const;

  XsAxes _axes;
  unsigned int _n = 0;
  /// Branch-major tabulated values
  std::vector<Real> _y;
};

/**
 * All group constants of one material, stored as a flat list of components. Components that
 * share a grid are interpolated together. The last state point is cached, so repeated
 * evaluations at the same state (e.g. a uniform temperature) only return the stored result.
 * The table also records the range of the state points it has been evaluated at, so that
 * callers can report sampling outside the tabulated range.
 */
class XsTable
{
public:
  /// @param order Total degree of least-squares fits
  XsTable(XsInterpolation type = XsInterpolation::NONE, unsigned int order = 2)
    : _type(type), _order(order)
  {
  }

  /**
   * Appends components tabulated on a grid. Call finalize() after the last one.
   * @param axes Knots of each axis, shared by all appended components
   * @param data data[c][b] is component c at branch b (row-major over the axes)
   * @param name Name of the components, used in error messages
   * @return Offset of the first appended component in the flat list
   */
  unsigned int
  addComponents(const XsAxes & axes, const std::vector<std::vector<Real>> & data, const std::string & name);

  /// Builds the interpolants; errors if the data do not suit the interpolation rule
  void finalize();

  /// Total number of components
  unsigned int size() const { return _values.size(); }

  /// Number of axes of the state point (0 before the first component is added)
  unsigned int dimension() const { return _lower.size(); }

  /**
   * Evaluates every component at state point x (one coordinate per axis). Derivatives along axis
   * d_axis are computed only when requested; derivatives() is stale after a call with
   * derivatives = false.
   */
  void evaluate(const std::vector<Real> & x, unsigned int d_axis, bool derivatives);

  const std::vector<Real> & values() const { return _values; }
  const std::vector<Real> & derivatives() const { return _dvalues; }

  /// Range of axis a covered by every component: the narrowest of the component grids
  Real lowerBound(unsigned int a) const { return _lower[a]; }
  Real upperBound(unsigned int a) const { return _upper[a]; }

  /**
   * Largest deviation of the table from its data at the knots for each named set of components,
   * relative to the largest magnitude of each component (zero for interpolating rules)
   */
  const std::vector<std::pair<std::string, Real>> & residuals() const { return _residuals; }

  /// Smallest and largest coordinates along each axis evaluated so far
  const std::vector<Real> & sampledMin() const { return _sampled_min; }
  const std::vector<Real> & sampledMax() const { return _sampled_max; }

private:
  struct Block
  {
    unsigned int offset;
    XsAxes axes;
    /// Component-major data, gathered until finalize()
    std::vector<std::vector<Real>> data;
    std::unique_ptr<XsBasis> basis;
  };

  XsInterpolation _type;
  unsigned int _order;
  std::vector<Block> _blocks;

  /// Name, offset and number of components of each addComponents() call
  struct Named
  {
    std::string name;
    unsigned int offset;
    unsigned int size;
  };
  std::vector<Named> _names;
  std::vector<std::pair<std::string, Real>> _residuals;

  std::vector<Real> _values;
  std::vector<Real> _dvalues;

  std::vector<Real> _lower;
  std::vector<Real> _upper;
  std::vector<Real> _sampled_min;
  std::vector<Real> _sampled_max;

  /// State of the cached result
  bool _cache_valid = false;
  bool _cache_has_derivatives = false;
  unsigned int _cache_d_axis = 0;
  std::vector<Real> _cache_x;
};
