#include "NearestPointDirectionalL2Projection.h"

registerMooseObject("MoltresApp", NearestPointDirectionalL2Projection);

InputParameters
NearestPointDirectionalL2Projection::validParams()
{
  InputParameters params =
      NearestPointDirectionalL2ProjectionBase<DirectionalL2Projection,
                                              ElementUserObject>::validParams();
  params.addClassDescription(
      "Weighted L2 projections of a variable over blocks onto continuous, piecewise-polynomial "
      "fields along one direction, one per point, each collecting the elements nearest to its "
      "point.");
  return params;
}

NearestPointDirectionalL2Projection::NearestPointDirectionalL2Projection(
    const InputParameters & parameters)
  : NearestPointDirectionalL2ProjectionBase<DirectionalL2Projection, ElementUserObject>(parameters)
{
}
