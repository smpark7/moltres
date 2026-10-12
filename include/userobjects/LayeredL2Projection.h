#pragma once

#include "LayeredL2ProjectionBase.h"
#include "ElementUserObject.h"

/**
 * Layered weighted L2 projection of a variable over blocks onto a continuous, piecewise-quadratic
 * 1-D field. Volume counterpart of LayeredAverage.
 */
class LayeredL2Projection : public LayeredL2ProjectionBase<ElementUserObject>
{
public:
  static InputParameters validParams();

  LayeredL2Projection(const InputParameters & parameters);
};
