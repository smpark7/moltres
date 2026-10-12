#pragma once

#include "NearestPointLayeredL2ProjectionBase.h"
#include "LayeredL2Projection.h"

/**
 * Layered L2 projections over blocks, one per point, each collecting the elements nearest
 * to its point. Counterpart of NearestPointLayeredAverage.
 */
class NearestPointLayeredL2Projection
  : public NearestPointLayeredL2ProjectionBase<LayeredL2Projection, ElementUserObject>
{
public:
  static InputParameters validParams();

  NearestPointLayeredL2Projection(const InputParameters & parameters);
};
