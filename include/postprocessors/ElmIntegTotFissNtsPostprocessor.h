#ifndef ELMINTEGTOTFISSNTSPOSTPROCESSOR_H
#define ELMINTEGTOTFISSNTSPOSTPROCESSOR_H

#include "ElementIntegralPostprocessor.h"
#include "MooseVariableInterface.h"

// Forward Declarations
class ElmIntegTotFissNtsPostprocessor;

template <>
InputParameters validParams<ElmIntegTotFissNtsPostprocessor>();

/**
 * This class computes the postprocessor value for the total number of
 * neutrons produced in one neutron generation from fission and delayed
 * neutron precursors if specified.
 * \f$ \sum_g \nu \Sigma_{g,f} \phi_g + \sum_i \lambda_i C_i \f$
 */
class ElmIntegTotFissNtsPostprocessor : public ElementIntegralPostprocessor
/* public MooseVariableInterface */
{
public:
  ElmIntegTotFissNtsPostprocessor(const InputParameters & parameters);

protected:
  virtual Real computeQpIntegral() override;

  // The number of neutron energy groups.
  int _num_groups;

  // The number of precursor groups.
  int _num_precursor_groups;

  // Whether to account for delayed neutrons
  bool _account_delayed;

  // nu Sigma_f material property
  const MaterialProperty<std::vector<Real>> & _nsf;

  // Decay constant material property
  const MaterialProperty<std::vector<Real>> & _decay_constant;

  std::vector<MooseVariableFEBase *> _vars;

  // Group flux variables
  std::vector<const VariableValue *> _group_fluxes;

  // Precursor concentration variables
  std::vector<const VariableValue *> _pre_concs;
};

#endif
