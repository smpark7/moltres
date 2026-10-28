#include "InScatter.h"

registerMooseObject("MoltresApp", InScatter);

InputParameters
InScatter::validParams()
{
  InputParameters params = QpCoefficientKernel::validParams();
  params += ScalarTransportBase::validParams();
  params.addRequiredParam<unsigned int>("group_number", "The current energy group");
  params.addRequiredParam<unsigned int>("num_groups", "The total numer of energy groups");
  params.addCoupledVar("temperature", "The temperature used to interpolate material properties");
  params.addRequiredCoupledVar("group_fluxes", "All the variables that hold the group fluxes. "
                                               "These MUST be listed by decreasing "
                                               "energy/increasing group number.");
  params.addParam<bool>(
      "sss2_input", true, "Whether serpent 2 was used to generate the input files.");
  return params;
}

InScatter::InScatter(const InputParameters & parameters)
  : QpCoefficientKernel(parameters),
    ScalarTransportBase(parameters),
    _gtransfxs(getMaterialProperty<std::vector<Real>>("gtransfxs")),
    _d_gtransfxs_d_temp(getMaterialProperty<std::vector<Real>>("d_gtransfxs_d_temp")),
    _inscatter_rate(getOptionalMaterialProperty<std::vector<Real>>("inscatter_rate")),
    _group(getParam<unsigned int>("group_number") - 1),
    _num_groups(getParam<unsigned int>("num_groups")),
    _temp_id(coupled("temperature")),
    _sss2_input(getParam<bool>("sss2_input"))
{
  unsigned int n = coupledComponents("group_fluxes");
  if (!(n == _num_groups))
  {
    mooseError("The number of coupled variables doesn't match the number of groups.");
  }
  _group_fluxes.resize(n);
  _flux_ids.resize(n);
  _idx.resize(n);
  for (unsigned int i = 0; i < _group_fluxes.size(); ++i)
  {
    _group_fluxes[i] = &coupledValue("group_fluxes", i);
    _flux_ids[i] = coupled("group_fluxes", i);
    if (_sss2_input)
      _idx[i] = i * _num_groups + _group;
    else
      _idx[i] = i + _group * _num_groups;
  }
}

Real
InScatter::precomputeQpResidual()
{
  if (_inscatter_rate)
    return -_inscatter_rate[_qp][_group];

  Real r = 0;
  for (unsigned int i = 0; i < _num_groups; ++i)
  {
    if (i == _group || _gtransfxs[_qp][_idx[i]] == 0.)
      continue;
    r += -_gtransfxs[_qp][_idx[i]] * computeConcentration((*_group_fluxes[i]), _qp);
  }

  return r;
}

bool
InScatter::precomputeOffDiagCoefficients(unsigned int jvar)
{
  for (unsigned int i = 0; i < _num_groups; ++i)
    if (jvar == _flux_ids[i])
    {
      for (_qp = 0; _qp < _qrule->n_points(); _qp++)
        _offdiag_coef[_qp] = -_gtransfxs[_qp][_idx[i]] *
                             computeConcentrationDerivativeFactor((*_group_fluxes[i]), _qp);
      return true;
    }

  if (jvar != _temp_id)
    return false;

  for (_qp = 0; _qp < _qrule->n_points(); _qp++)
  {
    Real jac = 0;
    for (unsigned int i = 0; i < _num_groups; ++i)
    {
      if (i == _group || _gtransfxs[_qp][_idx[i]] == 0.)
        continue;
      jac += -_d_gtransfxs_d_temp[_qp][_idx[i]] * computeConcentration((*_group_fluxes[i]), _qp);
    }
    _offdiag_coef[_qp] = jac;
  }
  return true;
}
