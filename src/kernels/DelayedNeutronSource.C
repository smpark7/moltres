#include "DelayedNeutronSource.h"

registerMooseObject("MoltresApp", DelayedNeutronSource);

InputParameters
DelayedNeutronSource::validParams()
{
  InputParameters params = QpCoefficientKernel::validParams();
  params += ScalarTransportBase::validParams();
  params.addRequiredParam<unsigned int>("num_precursor_groups", "The number of precursor groups.");
  params.addCoupledVar("temperature", "The temperature used to interpolate material properties");
  params.addRequiredCoupledVar("pre_concs", "All the variables that hold the precursor "
                                            "concentrations. These MUST be listed by increasing "
                                            "group number.");
  params.addCoupledVar("delayed_neutron_source",
                       "Delayed neutron source variable name. Optional parameter for reducing "
                       "variable data transfers between MultiApps with a consolidated delayed "
                       "neutron source variable.");
  params.addRequiredParam<unsigned int>("group_number","neutron energy group number for chi_d");
  return params;
}

DelayedNeutronSource::DelayedNeutronSource(const InputParameters & parameters)
  : QpCoefficientKernel(parameters),
    ScalarTransportBase(parameters),
    _decay_constant(getMaterialProperty<std::vector<Real>>("decay_constant")),
    _d_decay_constant_d_temp(getMaterialProperty<std::vector<Real>>("d_decay_constant_d_temp")),
    _chi_d(getMaterialProperty<std::vector<Real>>("chi_d")),
    _group(getParam<unsigned int>("group_number") - 1),
    _num_precursor_groups(getParam<unsigned int>("num_precursor_groups")),
    _temp_id(coupled("temperature")),
    _temp(coupledValue("temperature")),
    _has_delayed_source(isCoupled("delayed_neutron_source")),
    _delayed_source(isCoupled("delayed_neutron_source") ?
        coupledValue("delayed_neutron_source") : _zero)
{
  if (!_has_delayed_source)
  {
    unsigned int n = coupledComponents("pre_concs");
    if (!(n == _num_precursor_groups))
    {
      mooseError("The number of coupled variables doesn't match the number of groups.");
    }
    _pre_concs.resize(n);
    _pre_ids.resize(n);
    for (unsigned int i = 0; i < _pre_concs.size(); ++i)
    {
      _pre_concs[i] = &coupledValue("pre_concs", i);
      _pre_ids[i] = coupled("pre_concs", i);
    }
  }
}

Real
DelayedNeutronSource::precomputeQpResidual()
{
  if (_has_delayed_source)
    return -_chi_d[_qp][_group] * _delayed_source[_qp];
  Real r = 0;
  for (unsigned int i = 0; i < _num_precursor_groups; ++i)
    r += -_decay_constant[_qp][i] * computeConcentration((*_pre_concs[i]), _qp);

  return _chi_d[_qp][_group] * r;
}

bool
DelayedNeutronSource::precomputeOffDiagCoefficients(unsigned int jvar)
{
  if (_has_delayed_source)
    return false;

  for (unsigned int i = 0; i < _num_precursor_groups; ++i)
    if (jvar == _pre_ids[i])
    {
      for (_qp = 0; _qp < _qrule->n_points(); _qp++)
        _offdiag_coef[_qp] = -_chi_d[_qp][_group] * _decay_constant[_qp][i] *
                             computeConcentrationDerivativeFactor((*_pre_concs[i]), _qp);
      return true;
    }

  if (jvar != _temp_id)
    return false;

  for (_qp = 0; _qp < _qrule->n_points(); _qp++)
  {
    Real jac = 0;
    for (unsigned int i = 0; i < _num_precursor_groups; ++i)
      jac += -computeConcentration((*_pre_concs[i]), _qp) * _d_decay_constant_d_temp[_qp][i];
    _offdiag_coef[_qp] = _chi_d[_qp][_group] * jac;
  }
  return true;
}
