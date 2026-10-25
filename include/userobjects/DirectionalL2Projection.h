#pragma once

#include "DirectionalL2ProjectionBase.h"
#include "ElementUserObject.h"

/**
 * Directional weighted L2 projection of a variable over blocks onto a continuous,
 * piecewise-polynomial 1-D field. Volume counterpart of LayeredAverage.
 */
class DirectionalL2Projection : public DirectionalL2ProjectionBase<ElementUserObject>
{
public:
  static InputParameters validParams();

  DirectionalL2Projection(const InputParameters & parameters);
};
