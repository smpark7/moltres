#pragma once

#include "LayeredL2ProjectionBase.h"
#include "SideUserObject.h"

/**
 * Layered weighted L2 projection of a variable over boundaries onto a continuous,
 * piecewise-quadratic 1-D field. Side counterpart of LayeredSideAverage.
 */
class LayeredSideL2Projection : public LayeredL2ProjectionBase<SideUserObject>
{
public:
  static InputParameters validParams();

  LayeredSideL2Projection(const InputParameters & parameters);
};
