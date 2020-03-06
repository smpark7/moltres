#include "CoupledTemperatureOutflowBC.h"

registerMooseObject("MoltresApp", CoupledTemperatureOutflowBC);

template <>
InputParameters
validParams<CoupledTemperatureOutflowBC>()
{
  InputParameters params = validParams<CoupledOutflowBC>();
  return params;
}

CoupledTemperatureOutflowBC::CoupledTemperatureOutflowBC(const InputParameters & parameters)
  : CoupledOutflowBC(parameters),
    _rho(getMaterialProperty<Real>("rho")),
    _d_rho_d_u(getMaterialPropertyDerivative<Real>("rho", _var.name())),
    _cp(getMaterialProperty<Real>("cp")),
    _d_cp_d_u(getMaterialPropertyDerivative<Real>("cp", _var.name()))
{
}

void
CoupledTemperatureOutflowBC::initialSetup()
{
  validateNonlinearCoupling<Real>("rho");
  validateNonlinearCoupling<Real>("cp");
}

Real
CoupledTemperatureOutflowBC::computeQpResidual()
{
  return _rho[_qp] * _cp[_qp] * CoupledOutflowBC::computeQpResidual();
}

Real
CoupledTemperatureOutflowBC::computeQpJacobian()
{
  return _rho[_qp] * _cp[_qp] * CoupledOutflowBC::computeQpJacobian() +
        //  _d_rho_d_u[_qp] * _phi[_j][_qp] * _cp[_qp] * CoupledOutflowBC::computeQpResidual() +
         _rho[_qp] * _d_cp_d_u[_qp] * _phi[_j][_qp] * CoupledOutflowBC::computeQpResidual();
