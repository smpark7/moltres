#pragma once

#include "KernelGrad.h"
#include "ScalarTransportBase.h"

class GroupDiffusion : public KernelGrad, public ScalarTransportBase
{
public:
  GroupDiffusion(const InputParameters & parameters);

  static InputParameters validParams();

  /// Only the temperature block is nonzero; skip the element loops for all other variables
  virtual void computeOffDiagJacobian(unsigned int jvar) override;

protected:
  virtual RealGradient precomputeQpResidual() override;
  virtual RealGradient precomputeQpJacobian() override;
  virtual Real computeQpOffDiagJacobian(unsigned int jvar) override;

  /// Diffusion coefficient of this group at _qp, capped at the limit if one is set
  Real diffusionCoefficient() const;

  // Group constant MaterialProperty(s)
  const MaterialProperty<std::vector<Real>> & _diffcoef;
  const MaterialProperty<std::vector<Real>> & _d_diffcoef_d_temp;

  /// Neutron group number
  const unsigned int _group;

  /// Temperature variable ID
  const unsigned int _temp_id;

  /// Whether to set a diffusion coefficient limit
  const bool _set_limit;

  /// Diffusion coefficient limit value
  const Real _limit;
};
