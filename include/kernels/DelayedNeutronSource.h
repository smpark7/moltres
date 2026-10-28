#pragma once

#include "QpCoefficientKernel.h"
#include "ScalarTransportBase.h"

class DelayedNeutronSource : public QpCoefficientKernel, public ScalarTransportBase
{
public:
  DelayedNeutronSource(const InputParameters & parameters);

  static InputParameters validParams();

  /// The on-diagonal Jacobian is identically zero: skip the element loops that would only add
  /// zeros (they cost as much as a real kernel's Jacobian)
  virtual void computeJacobian() override {}

protected:
  virtual Real precomputeQpResidual() override;
  virtual bool precomputeOffDiagCoefficients(unsigned int jvar) override;

  // Group constant MaterialProperty(s)
  const MaterialProperty<std::vector<Real>> & _decay_constant;
  const MaterialProperty<std::vector<Real>> & _d_decay_constant_d_temp;
  const MaterialProperty<std::vector<Real>> &  _chi_d;

  /// Neutron group number
  const unsigned int _group;

  /// Number of precursor groups
  const unsigned int _num_precursor_groups;

  /// Temperature variable ID
  const unsigned int _temp_id;

  /// Temperature variable
  const VariableValue & _temp;

  /// Whether a delayed neutron source auxvariable is provided
  const bool _has_delayed_source;

  /// Delayed neutron source auxvariable
  const VariableValue & _delayed_source;

  /// Precursor variables
  std::vector<const VariableValue *> _pre_concs;

  /// Precursor variable IDs
  std::vector<unsigned int> _pre_ids;
};
