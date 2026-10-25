#include "DirectionalL2Projection.h"

registerMooseObject("MoltresApp", DirectionalL2Projection);

InputParameters
DirectionalL2Projection::validParams()
{
  InputParameters params = DirectionalL2ProjectionBase<ElementUserObject>::validParams();
  params.addClassDescription(
      "Weighted L2 projection of a variable over blocks onto a continuous, piecewise-polynomial "
      "field along one direction, with layers equal to the elements of the receiving 1-D mesh.");
  return params;
}

DirectionalL2Projection::DirectionalL2Projection(const InputParameters & parameters)
  : DirectionalL2ProjectionBase<ElementUserObject>(parameters)
{
}
