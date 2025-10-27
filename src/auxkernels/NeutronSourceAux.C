#include "NeutronSourceAux.h"

registerMooseObject("MoltresApp", NeutronSourceAux);

InputParameters
NeutronSourceAux::validParams()
{
  InputParameters params = AuxKernel::validParams();
  params += ScalarTransportBase::validParams();
  params.addRequiredParam<unsigned int>("num_groups", "The total number of energy groups");
  params.addRequiredCoupledVar("group_fluxes", "All the variables that hold the group fluxes. "
                                               "These MUST be listed by decreasing "
                                               "energy/increasing group number.");
  params.addCoupledVar(
      "temperature", 800, "The temperature used to interpolate material properties.");
  params.addParam<Real>("nt_scale", 1, "Scaling of the neutron fluxes to aid convergence.");
  params.addParam<PostprocessorName>("eigenvalue_scaling",
                        1.0,
                        "Artificial scaling factor for the fission source. Primarily for "
                        "introducing artificial reactivity to make super/subcritical systems "
                        "exactly critical or to simulate reactivity insertions/withdrawals.");
  return params;
}

NeutronSourceAux::NeutronSourceAux(const InputParameters & parameters)
  : AuxKernel(parameters),
    ScalarTransportBase(parameters),
    _nsf(getMaterialProperty<std::vector<Real>>("nsf")),
    _num_groups(getParam<unsigned int>("num_groups")),
    _temp(coupledValue("temperature")),
    _nt_scale(getParam<Real>("nt_scale")),
    _eigenvalue_scaling(getPostprocessorValue("eigenvalue_scaling"))
{
  _group_fluxes.resize(_num_groups);
  for (unsigned int i = 0; i < _group_fluxes.size(); ++i)
  {
    _group_fluxes[i] = &coupledValue("group_fluxes", i);
  }
}

Real
NeutronSourceAux::computeValue()
{
  Real r = 0;
  for (unsigned int i = 0; i < _num_groups; ++i)
  {
    r += _nsf[_qp][i] * computeConcentration((*_group_fluxes[i]), _qp) * _nt_scale;
  }

  if ((_eigenvalue_scaling > 1e-3))
    r /= _eigenvalue_scaling;

  return r;
}
