#ifndef COUPLEDTEMPERATUREOUTFLOWBC_H
#define COUPLEDTEMPERATUREOUTFLOWBC_H

#include "CoupledOutflowBC.h"

class CoupledTemperatureOutflowBC;

template<>
InputParameters validParams<CoupledTemperatureOutflowBC>();

class CoupledTemperatureOutflowBC : public CoupledOutflowBC
{
public:
  CoupledTemperatureOutflowBC(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() override;
  virtual Real computeQpJacobian() override;

  const MaterialProperty<Real> & _rho;
  const MaterialProperty<Real> & _d_rho_d_u;
  const MaterialProperty<Real> & _cp;
  const MaterialProperty<Real> & _d_cp_d_u;
};

#endif // COUPLEDTEMPERATUREOUTFLOWBC_H
