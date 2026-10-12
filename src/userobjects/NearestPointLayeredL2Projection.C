#include "NearestPointLayeredL2Projection.h"

registerMooseObject("MoltresApp", NearestPointLayeredL2Projection);

InputParameters
NearestPointLayeredL2Projection::validParams()
{
  InputParameters params =
      NearestPointLayeredL2ProjectionBase<LayeredL2Projection, ElementUserObject>::validParams();
  params.addClassDescription(
      "Weighted L2 projections of a variable over blocks onto continuous, piecewise-quadratic "
      "fields along one direction, one per point, each collecting the elements nearest to its "
      "point.");
  return params;
}

NearestPointLayeredL2Projection::NearestPointLayeredL2Projection(const InputParameters & parameters)
  : NearestPointLayeredL2ProjectionBase<LayeredL2Projection, ElementUserObject>(parameters)
{
}
