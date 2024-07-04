#include "ScalarTransportTimeDerivative.h"

registerMooseObject("MoltresApp", ScalarTransportTimeDerivative);

InputParameters
ScalarTransportTimeDerivative::validParams()
{
  InputParameters params = TimeKernel::validParams();
  params += ScalarTransportBase::validParams();
  params.addParam<bool>("lumping", false, "True for mass matrix lumping, false otherwise");
  params.addParam<Real>(
      "conc_scaling", 1, "The amount by which to scale the concentration variable.");
  return params;
}

ScalarTransportTimeDerivative::ScalarTransportTimeDerivative(const InputParameters & parameters)
  : TimeKernel(parameters),
    ScalarTransportBase(parameters),
    _lumping(getParam<bool>("lumping")),
    _conc_scaling(getParam<Real>("conc_scaling"))

{
}

Real
ScalarTransportTimeDerivative::computeQpResidual()
{
  return _test[_i][_qp] * _u_dot[_qp] * _conc_scaling;
}

Real
ScalarTransportTimeDerivative::computeQpJacobian()
{
  return _test[_i][_qp] * _du_dot_du[_qp] * _phi[_j][_qp] *
         _conc_scaling;
}
