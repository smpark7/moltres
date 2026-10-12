#pragma once

#include "Kernel.h"

class GammaHeatSource : public Kernel
{
public:
  GammaHeatSource(const InputParameters & parameters);

  static InputParameters validParams();

  /// The on-diagonal Jacobian is identically zero: skip the element loops that would only add
  /// zeros (they cost as much as a real kernel's Jacobian)
  virtual void computeJacobian() override {}

protected:
  virtual Real computeQpResidual() override;

  const PostprocessorValue & _average_fission_heat;
  const Function & _gamma;
};
