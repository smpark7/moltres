#include "PostprocessorFunctionInflowBC.h"
#include "Function.h"

registerMooseObject("MoltresApp", PostprocessorFunctionInflowBC);

InputParameters
PostprocessorFunctionInflowBC::validParams()
{
  InputParameters params = IntegratedBC::validParams();
  params.addRequiredParam<FunctionName>("vel_x_func", "The x velocity function");
  params.addRequiredParam<FunctionName>("vel_y_func", "The y velocity function");
  params.addRequiredParam<FunctionName>("vel_z_func", "The z velocity function");
  params.addRequiredParam<PostprocessorName>(
      "postprocessor", "The postprocessor from which to derive the inlet concentration.");
  params.addParam<Real>("scale", 1, "The amount to scale the postprocessor value by");
  params.addParam<Real>("offset", 0, "The amount to offset the scaled postprocessor by");
  return params;
}

PostprocessorFunctionInflowBC::PostprocessorFunctionInflowBC(const InputParameters & parameters)
  : IntegratedBC(parameters),
    _vel_x_func(getFunction("vel_x_func")),
    _vel_y_func(getFunction("vel_y_func")),
    _vel_z_func(getFunction("vel_z_func")),
    _pp_value(getPostprocessorValue("postprocessor")),
    _scale(getParam<Real>("scale")),
    _offset(getParam<Real>("offset"))
{
}

Real
PostprocessorFunctionInflowBC::computeQpResidual()
{
  RealVectorValue v = {_vel_x_func.value(_t, _q_point[_qp]),
                       _vel_y_func.value(_t, _q_point[_qp]),
                       _vel_z_func.value(_t, _q_point[_qp])};
  Real vdotn = v * _normals[_qp];
  Real inlet_conc = _scale * _pp_value + _offset;

  return _test[_i][_qp] * inlet_conc * vdotn;
}

Real
PostprocessorFunctionInflowBC::computeQpJacobian()
{
  return 0.;
}
