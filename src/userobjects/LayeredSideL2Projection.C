#include "LayeredSideL2Projection.h"

registerMooseObject("MoltresApp", LayeredSideL2Projection);

InputParameters
LayeredSideL2Projection::validParams()
{
  InputParameters params = LayeredL2ProjectionBase<SideUserObject>::validParams();
  params.addClassDescription(
      "Weighted L2 projection of a variable over boundaries onto a continuous, "
      "piecewise-quadratic field along one direction, with layers equal to the elements of the "
      "receiving 1-D mesh.");
  return params;
}

LayeredSideL2Projection::LayeredSideL2Projection(const InputParameters & parameters)
  : LayeredL2ProjectionBase<SideUserObject>(parameters)
{
}
