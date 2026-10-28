#include "ElmIntegTotFissHeatPostprocessor.h"

registerMooseObject("MoltresApp", ElmIntegTotFissHeatPostprocessor);

InputParameters
ElmIntegTotFissHeatPostprocessor::validParams()
{
  InputParameters params = ElmIntegTotFissPostprocessor::validParams();
  return params;
}

ElmIntegTotFissHeatPostprocessor::ElmIntegTotFissHeatPostprocessor(
    const InputParameters & parameters)
  : ElmIntegTotFissPostprocessor(parameters),
    _fisse(getMaterialProperty<std::vector<Real>>("fisse")),
    _fission_heat_rate(getOptionalMaterialProperty<Real>("fission_heat_rate"))
{
}

Real
ElmIntegTotFissHeatPostprocessor::computeQpIntegral()
{
  if (_fission_heat_rate)
    return _fission_heat_rate[_qp] * _nt_scale;
  return ElmIntegTotFissPostprocessor::computeQpIntegral();
}

Real
ElmIntegTotFissHeatPostprocessor::computeFluxMultiplier(int index)
{
  return _fisse[_qp][index] * _fissxs[_qp][index];
}
