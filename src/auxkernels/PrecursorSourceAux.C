#include "PrecursorSourceAux.h"

registerMooseObject("MoltresApp", PrecursorSourceAux);

InputParameters
PrecursorSourceAux::validParams()
{
  InputParameters params = AuxKernel::validParams();
  params += ScalarTransportBase::validParams();
  params.addRequiredParam<unsigned int>("num_groups", "The total numer of energy groups");
  params.addRequiredCoupledVar("group_fluxes", "All the variables that hold the group fluxes. "
                                               "These MUST be listed by decreasing "
                                               "energy/increasing group number.");
  params.addCoupledVar("neutron_source",
                       "Fission neutron source variable name. Optional parameter for reducing "
                       "variable data transfers between MultiApps with a consolidated fission "
                       "neutron source variable.");
  params.addParam<unsigned int>("precursor_group_number",
                                "What precursor group this kernel is acting on.");
  params.addParam<Real>("prec_scale", 1, "The factor by which the neutron fluxes are scaled.");
  params.addParam<PostprocessorName>("eigenvalue_scaling",
                        1.0,
                        "Artificial scaling factor for the fission source. Primarily for "
                        "introducing artificial reactivity to make super/subcritical systems "
                        "exactly critical or to simulate reactivity insertions/withdrawals.");
  return params;
}

PrecursorSourceAux::PrecursorSourceAux(const InputParameters & parameters)
  : AuxKernel(parameters),
    ScalarTransportBase(parameters),
    _nsf(getMaterialProperty<std::vector<Real>>("nsf")),
    _num_groups(getParam<unsigned int>("num_groups")),
    _beta_eff(getMaterialProperty<std::vector<Real>>("beta_eff")),
    _precursor_group(getParam<unsigned int>("precursor_group_number") - 1),
    _prec_scale(getParam<Real>("prec_scale")),
    _eigenvalue_scaling(getPostprocessorValue("eigenvalue_scaling")),
    _has_neutron_source(isCoupled("neutron_source")),
    _neutron_source(isCoupled("neutron_source") ?
        coupledValue("neutron_source") : _zero)
{
  if (!_has_neutron_source)
  {
    _group_fluxes.resize(_num_groups);
    _flux_ids.resize(_num_groups);
    for (unsigned int i = 0; i < _group_fluxes.size(); ++i)
    {
      _group_fluxes[i] = &coupledValue("group_fluxes", i);
      _flux_ids[i] = coupled("group_fluxes", i);
    }
  }
}

Real
PrecursorSourceAux::computeValue()
{
  Real r = 0;
  if (_has_neutron_source)
    r += _beta_eff[_qp][_precursor_group] * _neutron_source[_qp] * _prec_scale;
  else
    for (unsigned int i = 0; i < _num_groups; ++i)
      r += _beta_eff[_qp][_precursor_group] * _nsf[_qp][i] *
           computeConcentration((*_group_fluxes[i]), _qp) * _prec_scale;

  if ((_eigenvalue_scaling != 1.0))
    r /= _eigenvalue_scaling;

  return r;
}
