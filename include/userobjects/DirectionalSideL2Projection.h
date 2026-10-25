#pragma once

#include "DirectionalL2ProjectionBase.h"
#include "SideUserObject.h"

/**
 * Directional weighted L2 projection of a variable over boundaries onto a continuous,
 * piecewise-polynomial 1-D field. Side counterpart of LayeredSideAverage.
 */
class DirectionalSideL2Projection : public DirectionalL2ProjectionBase<SideUserObject>
{
public:
  static InputParameters validParams();

  DirectionalSideL2Projection(const InputParameters & parameters);
};
