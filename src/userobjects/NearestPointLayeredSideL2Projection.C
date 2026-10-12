#include "NearestPointLayeredSideL2Projection.h"

registerMooseObject("MoltresApp", NearestPointLayeredSideL2Projection);

InputParameters
NearestPointLayeredSideL2Projection::validParams()
{
  InputParameters params =
      NearestPointLayeredL2ProjectionBase<LayeredSideL2Projection, SideUserObject>::validParams();
  params.addClassDescription(
      "Weighted L2 projections of a variable over boundaries onto continuous, piecewise-quadratic "
      "fields along one direction, one per point, each collecting the elements nearest to its "
      "point.");
  return params;
}

NearestPointLayeredSideL2Projection::NearestPointLayeredSideL2Projection(
    const InputParameters & parameters)
  : NearestPointLayeredL2ProjectionBase<LayeredSideL2Projection, SideUserObject>(parameters)
{
}
