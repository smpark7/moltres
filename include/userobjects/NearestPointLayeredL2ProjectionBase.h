#pragma once

#include "NearestPointBase.h"

/**
 * One layered L2 projection per point; each element contributes to the projection of the point
 * nearest to its vertex average (as NearestPointLayeredAverage does for LayeredAverage). Use
 * with one point per receiving 1-D channel.
 */
template <typename UserObjectType, typename BaseType>
class NearestPointLayeredL2ProjectionBase : public NearestPointBase<UserObjectType, BaseType>
{
public:
  static InputParameters validParams()
  {
    return NearestPointBase<UserObjectType, BaseType>::validParams();
  }

  NearestPointLayeredL2ProjectionBase(const InputParameters & parameters)
    : NearestPointBase<UserObjectType, BaseType>(parameters)
  {
    // The per-point projections are not managed by MOOSE, so declare the dependency on the
    // projected variable here, so that it is prepared on every element this object visits.
    this->coupledValue("variable");
  }

  /// Nodes of each point's projected field, placed at that point across the direction
  virtual const std::vector<Point> spatialPoints() const override
  {
    std::vector<Point> points;
    for (const auto i : index_range(this->_points))
    {
      const auto & uo = *this->_user_objects[i];
      for (const auto & node : uo.spatialPoints())
      {
        Point pt = this->_points[i];
        pt(uo.direction()) = node(uo.direction());
        points.push_back(pt);
      }
    }
    return points;
  }
};
