#include "CoupledFissionKernel.h"

registerMooseObject("MoltresApp", CoupledFissionKernel);

InputParameters
CoupledFissionKernel::validParams()
{
  InputParameters params = QpCoefficientKernel::validParams();
  params += ScalarTransportBase::validParams();
  params.addRequiredParam<unsigned int>("group_number", "The current energy group");
  params.addRequiredParam<unsigned int>("num_groups", "The total numer of energy groups");
  params.addRequiredCoupledVar("temperature",
                               "The temperature used to interpolate material properties");
  params.addRequiredCoupledVar("group_fluxes", "All the variables that hold the group fluxes. "
                                               "These MUST be listed by decreasing "
                                               "energy/increasing group number.");
  params.addRequiredParam<bool>("account_delayed", "Whether to account for delayed neutrons.");
  params.addParam<PostprocessorName>("eigenvalue_scaling", 1.0,
                                     "Artificial scaling factor for the fission "
                                     "source. Primarily introduced to make "
                                     "super/sub-critical systems exactly critical "
                                     "for the CNRS benchmark.");
  return params;
}

CoupledFissionKernel::CoupledFissionKernel(const InputParameters & parameters)
  : QpCoefficientKernel(parameters),
    ScalarTransportBase(parameters),
    _nsf(getMaterialProperty<std::vector<Real>>("nsf")),
    _d_nsf_d_temp(getMaterialProperty<std::vector<Real>>("d_nsf_d_temp")),
    _chi_t(getMaterialProperty<std::vector<Real>>("chi_t")),
    _chi_p(getMaterialProperty<std::vector<Real>>("chi_p")),
    _d_chi_t_d_temp(getMaterialProperty<std::vector<Real>>("d_chi_t_d_temp")),
    _d_chi_p_d_temp(getMaterialProperty<std::vector<Real>>("d_chi_p_d_temp")),
    _beta(getMaterialProperty<Real>("beta")),
    _d_beta_d_temp(getMaterialProperty<Real>("d_beta_d_temp")),
    _fission_neutron_rate(getOptionalMaterialProperty<Real>("fission_neutron_rate")),
    _group(getParam<unsigned int>("group_number") - 1),
    _num_groups(getParam<unsigned int>("num_groups")),
    _temp_id(coupled("temperature")),
    _temp(coupledValue("temperature")),
    _account_delayed(getParam<bool>("account_delayed")),
    _eigenvalue_scaling(getPostprocessorValue("eigenvalue_scaling"))
{
  unsigned int n = coupledComponents("group_fluxes");
  if (!(n == _num_groups))
  {
    mooseError("The number of coupled variables doesn't match the number of groups.");
  }
  _group_fluxes.resize(n);
  _flux_ids.resize(n);
  for (unsigned int i = 0; i < _group_fluxes.size(); ++i)
  {
    _group_fluxes[i] = &coupledValue("group_fluxes", i);
    _flux_ids[i] = coupled("group_fluxes", i);
  }
}

Real
CoupledFissionKernel::spectrumFactor() const
{
  Real factor = _account_delayed ? (1. - _beta[_qp]) * _chi_p[_qp][_group] : _chi_t[_qp][_group];
  if (_eigenvalue_scaling != 1.0)
    factor /= _eigenvalue_scaling;
  return factor;
}

Real
CoupledFissionKernel::precomputeQpResidual()
{
  Real r = 0;
  if (_fission_neutron_rate)
    r = -_fission_neutron_rate[_qp];
  else
    for (unsigned int i = 0; i < _num_groups; ++i)
      r += -_nsf[_qp][i] * computeConcentration((*_group_fluxes[i]), _qp);

  return r * spectrumFactor();
}

Real
CoupledFissionKernel::precomputeQpJacobian()
{
  return -_nsf[_qp][_group] *
         computeConcentrationDerivative((*_group_fluxes[_group]), _phi, _j, _qp) *
         spectrumFactor();
}

bool
CoupledFissionKernel::precomputeOffDiagCoefficients(unsigned int jvar)
{
  for (unsigned int i = 0; i < _num_groups; ++i)
    if (jvar == _flux_ids[i])
    {
      for (_qp = 0; _qp < _qrule->n_points(); _qp++)
        _offdiag_coef[_qp] = -_nsf[_qp][i] *
                             computeConcentrationDerivativeFactor((*_group_fluxes[i]), _qp) *
                             spectrumFactor();
      return true;
    }

  if (jvar != _temp_id)
    return false;

  for (_qp = 0; _qp < _qrule->n_points(); _qp++)
  {
    Real jac = 0;
    for (unsigned int i = 0; i < _num_groups; ++i)
    {
      if (_account_delayed)
        jac += -computeConcentration((*_group_fluxes[i]), _qp) *
               (_d_chi_p_d_temp[_qp][_group] * _nsf[_qp][i] * (1. - _beta[_qp]) +
                _chi_p[_qp][_group] * _d_nsf_d_temp[_qp][i] * (1. - _beta[_qp]) +
                _chi_p[_qp][_group] * _nsf[_qp][i] * -_d_beta_d_temp[_qp]);
      else
        jac += -computeConcentration((*_group_fluxes[i]), _qp) *
               (_d_chi_t_d_temp[_qp][_group] * _nsf[_qp][i] +
                _chi_t[_qp][_group] * _d_nsf_d_temp[_qp][i]);
    }
    if (_eigenvalue_scaling != 1.0)
      jac /= _eigenvalue_scaling;
    _offdiag_coef[_qp] = jac;
  }
  return true;
}
