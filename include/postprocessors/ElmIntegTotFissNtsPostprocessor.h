#pragma once

#include "ElementIntegralPostprocessor.h"
#include "MooseVariableInterface.h"

class ElmIntegTotFissNtsPostprocessor : public ElementIntegralPostprocessor
/* public MooseVariableInterface */
{
public:
  ElmIntegTotFissNtsPostprocessor(const InputParameters & parameters);

  static InputParameters validParams();

protected:
  virtual Real computeQpIntegral() override;

  unsigned int _num_groups;
  unsigned int _num_precursor_groups;
  bool _account_delayed;
  const MaterialProperty<std::vector<Real>> & _nsf;
  const MaterialProperty<Real> & _beta;
  const MaterialProperty<std::vector<Real>> & _decay_constant;
  std::vector<MooseVariableFEBase *> _vars;
  std::vector<const VariableValue *> _group_fluxes;
  std::vector<const VariableValue *> _pre_concs;
};
