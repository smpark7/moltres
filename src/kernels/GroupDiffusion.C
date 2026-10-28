#include "GroupDiffusion.h"

registerMooseObject("MoltresApp", GroupDiffusion);

InputParameters
GroupDiffusion::validParams()
{
  InputParameters params = KernelGrad::validParams();
  params += ScalarTransportBase::validParams();
  params.addRequiredParam<unsigned int>("group_number",
                                        "The group for which this kernel controls diffusion");
  params.addCoupledVar("temperature",
                       "The temperature used to interpolate the diffusion coefficient");
  params.addParam<bool>("set_diffcoef_limit",
      false,
      "Replaces all diffusion coefficient values above the specified limit to the limit value. "
      "Primarily helps with stabilizing drift coefficients in void regions.");
  params.addParam<Real>("diffcoef_limit", 5.0, "Maximum diffusion coefficient value limit.");
  return params;
}

GroupDiffusion::GroupDiffusion(const InputParameters & parameters)
  : KernelGrad(parameters),
    ScalarTransportBase(parameters),
    _diffcoef(getMaterialProperty<std::vector<Real>>("diffcoef")),
    _d_diffcoef_d_temp(getMaterialProperty<std::vector<Real>>("d_diffcoef_d_temp")),
    _group(getParam<unsigned int>("group_number") - 1),
    _temp_id(coupled("temperature")),
    _set_limit(getParam<bool>("set_diffcoef_limit")),
    _limit(getParam<Real>("diffcoef_limit"))
{
}

Real
GroupDiffusion::diffusionCoefficient() const
{
  if (_set_limit && _diffcoef[_qp][_group] > _limit)
    return _limit;
  return _diffcoef[_qp][_group];
}

RealGradient
GroupDiffusion::precomputeQpResidual()
{
  return diffusionCoefficient() * computeConcentrationGradient(_u, _grad_u, _qp);
}

RealGradient
GroupDiffusion::precomputeQpJacobian()
{
  return diffusionCoefficient() *
         computeConcentrationGradientDerivative(_u, _grad_u, _phi, _grad_phi, _j, _qp);
}

void
GroupDiffusion::computeOffDiagJacobian(const unsigned int jvar)
{
  if (jvar == _var.number() || jvar == _temp_id)
    KernelGrad::computeOffDiagJacobian(jvar);
}

Real
GroupDiffusion::computeQpOffDiagJacobian(unsigned int jvar)
{
  if (_set_limit && _diffcoef[_qp][_group] > _limit)
    return 0;
  if (jvar == _temp_id)
    return _d_diffcoef_d_temp[_qp][_group] * _phi[_j][_qp] * _grad_test[_i][_qp] *
           computeConcentrationGradient(_u, _grad_u, _qp);
  else
    return 0;
}
