#pragma once

#include "AuxKernel.h"
#include "ScalarTransportBase.h"

class NeutronSourceAux : public AuxKernel, public ScalarTransportBase
{
public:
  NeutronSourceAux(const InputParameters & parameters);

  static InputParameters validParams();

protected:
  virtual Real computeValue() override;

  // Group constant MaterialProperty(s)
  const MaterialProperty<std::vector<Real>> & _nsf;

  /// Number of neutron groups
  unsigned int _num_groups;

  /// Temperature variable
  const VariableValue & _temp;

  /// Neutron flux scaling factor
  Real _nt_scale;

  /// Eigenvalue scaling factor
  const PostprocessorValue & _eigenvalue_scaling;

  /// Group flux variables
  std::vector<const VariableValue *> _group_fluxes;
};
