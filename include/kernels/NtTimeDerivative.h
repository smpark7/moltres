#pragma once

#include "ScalarTransportTimeDerivative.h"

class NtTimeDerivative : public ScalarTransportTimeDerivative
{
public:
  NtTimeDerivative(const InputParameters & parameters);

  static InputParameters validParams();

  /// The residual and Jacobian are evaluated once per quadrature point (and shape function)
  /// rather than for every test function
  virtual void computeResidual() override;
  virtual void computeJacobian() override;
  /// Only the temperature block is nonzero; skip the element loops for all other variables
  virtual void computeOffDiagJacobian(unsigned int jvar) override;

protected:
  virtual Real computeQpResidual() override;
  virtual Real computeQpJacobian() override;
  virtual Real computeQpOffDiagJacobian(unsigned int jvar) override;

  // Group constant MaterialProperty(s)
  const MaterialProperty<std::vector<Real>> & _recipvel;
  const MaterialProperty<std::vector<Real>> & _d_recipvel_d_temp;

  /// Neutron group number
  const unsigned int _group;

  /// Temperature variable ID
  const unsigned int _temp_id;
};
