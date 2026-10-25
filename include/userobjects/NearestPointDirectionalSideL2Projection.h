#pragma once

#include "NearestPointDirectionalL2ProjectionBase.h"
#include "DirectionalSideL2Projection.h"

/**
 * Directional side L2 projections over boundaries, one per point, each collecting the elements
 * nearest to its point. Counterpart of NearestPointLayeredSideAverage.
 */
class NearestPointDirectionalSideL2Projection
  : public NearestPointDirectionalL2ProjectionBase<DirectionalSideL2Projection, SideUserObject>
{
public:
  static InputParameters validParams();

  NearestPointDirectionalSideL2Projection(const InputParameters & parameters);
};
