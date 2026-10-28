#pragma once

#include "GeneralPostprocessor.h"

class FissionWeightVector;

/**
 * W . u for the weight vector W of a FissionWeightVector and the current solution u. Costs one
 * parallel dot product, so it can run on linear in place of an element-integral postprocessor.
 */
class WeightVectorDotProduct : public GeneralPostprocessor
{
public:
  static InputParameters validParams();

  WeightVectorDotProduct(const InputParameters & parameters);

  virtual void initialize() override {}
  virtual void execute() override;
  virtual Real getValue() const override { return _value; }

protected:
  const FissionWeightVector & _weight_vector;
  Real _value;
};
