#pragma once

#include "AuxKernel.h"
#include "ScalarTransportBase.h"

class PrecursorSourceAux : public AuxKernel, public ScalarTransportBase
{
public:
  PrecursorSourceAux(const InputParameters & parameters);

  static InputParameters validParams();

protected:
  virtual Real computeValue() override;

  const MaterialProperty<std::vector<Real>> & _nsf;
  unsigned int _num_groups;
  const MaterialProperty<std::vector<Real>> & _beta_eff;
  unsigned int _precursor_group;
  std::vector<const VariableValue *> _group_fluxes;
  std::vector<unsigned int> _flux_ids;
  Real _prec_scale;
  const PostprocessorValue & _eigenvalue_scaling;
  const bool _has_neutron_source;
  const VariableValue & _neutron_source;
};
