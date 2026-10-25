#pragma once

#include "NearestPointDirectionalL2ProjectionBase.h"
#include "DirectionalL2Projection.h"

/**
 * Directional L2 projections over blocks, one per point, each collecting the elements nearest
 * to its point. Counterpart of NearestPointLayeredAverage.
 */
class NearestPointDirectionalL2Projection
  : public NearestPointDirectionalL2ProjectionBase<DirectionalL2Projection, ElementUserObject>
{
public:
  static InputParameters validParams();

  NearestPointDirectionalL2Projection(const InputParameters & parameters);
};
