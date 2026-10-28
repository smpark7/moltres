#include "SigmaR.h"

registerMooseObject("MoltresApp", SigmaR);

InputParameters
SigmaR::validParams()
{
  InputParameters params = QpCoefficientKernel::validParams();
  params += ScalarTransportBase::validParams();
  params.addRequiredParam<unsigned int>("group_number", "The current energy group.");
  params.addCoupledVar("temperature", "The temperature used to interpolate material properties");
  return params;
}

SigmaR::SigmaR(const InputParameters & parameters)
  : QpCoefficientKernel(parameters),
    ScalarTransportBase(parameters),
    _remxs(getMaterialProperty<std::vector<Real>>("remxs")),
    _d_remxs_d_temp(getMaterialProperty<std::vector<Real>>("d_remxs_d_temp")),
    _group(getParam<unsigned int>("group_number") - 1),
    _temp_id(coupled("temperature"))
{
}

Real
SigmaR::precomputeQpResidual()
{
  return _remxs[_qp][_group] * computeConcentration(_u, _qp);
}

Real
SigmaR::precomputeQpJacobian()
{
  return _remxs[_qp][_group] * computeConcentrationDerivative(_u, _phi, _j, _qp);
}

bool
SigmaR::precomputeOffDiagCoefficients(unsigned int jvar)
{
  if (jvar != _temp_id)
    return false;
  for (_qp = 0; _qp < _qrule->n_points(); _qp++)
    _offdiag_coef[_qp] = _d_remxs_d_temp[_qp][_group] * computeConcentration(_u, _qp);
  return true;
}
