#include "VelocityFunctionAdvection.h"
#include "Function.h"

registerMooseObject("MoltresApp", VelocityFunctionAdvection);

template<>
InputParameters
validParams<VelocityFunctionAdvection>()
{
  InputParameters params = validParams<Kernel>();
  params.addRequiredParam<FunctionName>("vel_x_func", "The x velocity function");
  params.addRequiredParam<FunctionName>("vel_y_func", "The y velocity function");
  params.addRequiredParam<FunctionName>("vel_z_func", "The z velocity function");
  return params;
}

VelocityFunctionAdvection::VelocityFunctionAdvection(const InputParameters & parameters)
  : Kernel(parameters),
    _vel_x_func(getFunction("vel_x_func")),
    _vel_y_func(getFunction("vel_y_func")),
    _vel_z_func(getFunction("vel_z_func"))
{
}

Real
VelocityFunctionAdvection::computeQpResidual()
{
  RealVectorValue v = {_vel_x_func.value(_t, _q_point[_qp]),
                       _vel_y_func.value(_t, _q_point[_qp]),
                       _vel_z_func.value(_t, _q_point[_qp])};

  return -_grad_test[_i][_qp] * v * _u[_qp];
}

Real
VelocityFunctionAdvection::computeQpJacobian()
{
  RealVectorValue v = {_vel_x_func.value(_t, _q_point[_qp]),
                       _vel_y_func.value(_t, _q_point[_qp]),
                       _vel_z_func.value(_t, _q_point[_qp])};

  return -_grad_test[_i][_qp] * v * _phi[_j][_qp];
}
