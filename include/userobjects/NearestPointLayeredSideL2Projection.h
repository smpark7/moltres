#pragma once

#include "NearestPointLayeredL2ProjectionBase.h"
#include "LayeredSideL2Projection.h"

/**
 * Layered side L2 projections over boundaries, one per point, each collecting the elements
 * nearest to its point. Counterpart of NearestPointLayeredSideAverage.
 */
class NearestPointLayeredSideL2Projection
  : public NearestPointLayeredL2ProjectionBase<LayeredSideL2Projection, SideUserObject>
{
public:
  static InputParameters validParams();

  NearestPointLayeredSideL2Projection(const InputParameters & parameters);
};
