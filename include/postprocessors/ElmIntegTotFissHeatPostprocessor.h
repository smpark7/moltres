#pragma once

#include "ElmIntegTotFissPostprocessor.h"

class ElmIntegTotFissHeatPostprocessor : public ElmIntegTotFissPostprocessor
{
public:
  ElmIntegTotFissHeatPostprocessor(const InputParameters & parameters);

  static InputParameters validParams();

protected:
  virtual Real computeQpIntegral() override;
  virtual Real computeFluxMultiplier(int index) override;

  const MaterialProperty<std::vector<Real>> & _fisse;
  /// Group-summed fission heat rate from NeutronReactionRateMaterial, if one is present
  const OptionalMaterialProperty<Real> & _fission_heat_rate;
};
