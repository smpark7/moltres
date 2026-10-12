#include "LayeredL2Projection.h"

registerMooseObject("MoltresApp", LayeredL2Projection);

InputParameters
LayeredL2Projection::validParams()
{
  InputParameters params = LayeredL2ProjectionBase<ElementUserObject>::validParams();
  params.addClassDescription(
      "Weighted L2 projection of a variable over blocks onto a continuous, piecewise-quadratic "
      "field along one direction, with layers equal to the elements of the receiving 1-D mesh.");
  return params;
}

LayeredL2Projection::LayeredL2Projection(const InputParameters & parameters)
  : LayeredL2ProjectionBase<ElementUserObject>(parameters)
{
}
