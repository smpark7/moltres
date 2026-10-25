#include "NearestPointDirectionalSideL2Projection.h"

registerMooseObject("MoltresApp", NearestPointDirectionalSideL2Projection);

InputParameters
NearestPointDirectionalSideL2Projection::validParams()
{
  InputParameters params = NearestPointDirectionalL2ProjectionBase<DirectionalSideL2Projection,
                                                                   SideUserObject>::validParams();
  params.addClassDescription(
      "Weighted L2 projections of a variable over boundaries onto continuous, piecewise-polynomial "
      "fields along one direction, one per point, each collecting the elements nearest to its "
      "point.");
  return params;
}

NearestPointDirectionalSideL2Projection::NearestPointDirectionalSideL2Projection(
    const InputParameters & parameters)
  : NearestPointDirectionalL2ProjectionBase<DirectionalSideL2Projection, SideUserObject>(parameters)
{
}
