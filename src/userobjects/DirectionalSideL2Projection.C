#include "DirectionalSideL2Projection.h"

registerMooseObject("MoltresApp", DirectionalSideL2Projection);

InputParameters
DirectionalSideL2Projection::validParams()
{
  InputParameters params = DirectionalL2ProjectionBase<SideUserObject>::validParams();
  params.addClassDescription(
      "Weighted L2 projection of a variable over boundaries onto a continuous, "
      "piecewise-polynomial field along one direction, with layers equal to the elements of the "
      "receiving 1-D mesh.");
  return params;
}

DirectionalSideL2Projection::DirectionalSideL2Projection(const InputParameters & parameters)
  : DirectionalL2ProjectionBase<SideUserObject>(parameters)
{
}
