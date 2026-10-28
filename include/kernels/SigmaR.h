#pragma once

#include "QpCoefficientKernel.h"
#include "ScalarTransportBase.h"

class SigmaR : public QpCoefficientKernel, public ScalarTransportBase
{
public:
  SigmaR(const InputParameters & parameters);

  static InputParameters validParams();

protected:
  virtual Real precomputeQpResidual() override;
  virtual Real precomputeQpJacobian() override;
  virtual bool precomputeOffDiagCoefficients(unsigned int jvar) override;

  // Group constant MaterialProperty(s)
  const MaterialProperty<std::vector<Real>> & _remxs;
  const MaterialProperty<std::vector<Real>> & _d_remxs_d_temp;

  /// Neutron group number
  const unsigned int _group;

  /// Temperature variable ID
  const unsigned int _temp_id;
};
