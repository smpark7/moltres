#include "FissionWeightVector.h"
#include "MaterialBase.h"
#include "MaterialWarehouse.h"

registerMooseObject("MoltresApp", FissionWeightVector);

Threads::spin_mutex FissionWeightVector::_mutex;

InputParameters
FissionWeightVector::validParams()
{
  InputParameters params = ElementUserObject::validParams();
  params += ScalarTransportBase::validParams();
  params.addClassDescription(
      "Weight vector W of the fission heat or fission neutron production integral, such that the "
      "integral equals W . u. Use with WeightVectorDotProduct.");
  params.addRequiredCoupledVar(
      "group_fluxes",
      "The group fluxes. MUST be arranged by decreasing energy/increasing group number.");
  params.addRequiredParam<unsigned int>("num_groups", "The number of energy groups.");
  params.addRequiredParam<MooseEnum>(
      "quantity",
      MooseEnum("fission_heat fission_neutrons"),
      "fission_heat: sum_g fisse_g fissxs_g phi_g * nt_scale, as ElmIntegTotFissHeatPostprocessor. "
      "fission_neutrons: sum_g nsf_g phi_g, as ElmIntegTotFissNtsPostprocessor with "
      "account_delayed = false.");
  params.addParam<Real>("nt_scale", 1, "Scaling of the neutron fluxes (fission_heat only).");
  // Rebuild after the timestep_begin transfers and aux kernels that set the temperatures
  params.set<ExecFlagEnum>("execute_on") = {EXEC_INITIAL, EXEC_TIMESTEP_BEGIN};
  params.set<bool>("force_postaux") = true;
  return params;
}

FissionWeightVector::FissionWeightVector(const InputParameters & parameters)
  : ElementUserObject(parameters),
    ScalarTransportBase(parameters),
    _num_groups(getParam<unsigned int>("num_groups")),
    _heat(getParam<MooseEnum>("quantity") == "fission_heat"),
    _nt_scale(getParam<Real>("nt_scale")),
    _xs(getMaterialProperty<std::vector<Real>>(_heat ? "fissxs" : "nsf")),
    _fisse(_heat ? &getMaterialProperty<std::vector<Real>>("fisse") : nullptr),
    _sys(getVar("group_fluxes", 0)->sys()),
    _weights(_sys.addVector("fission_weights_" + name(), false, PARALLEL)),
    _built(false)
{
  // WeightVectorDotProduct builds W on demand with this flag if it is evaluated first, e.g. by
  // EigenProblem's initial scaling of the eigenvector
  if (!getExecuteOnEnum().contains(EXEC_INITIAL))
    paramError("execute_on", "Must include initial.");
  if (getParam<bool>("use_exp_form"))
    paramError("use_exp_form",
               "The fission integral is not linear in the solution when the fluxes are in "
               "exponential form.");
  if (coupledComponents("group_fluxes") != _num_groups)
    paramError("group_fluxes", "There must be one group flux variable per energy group.");
  if (isParamSetByUser("nt_scale") && !_heat)
    paramError("nt_scale", "nt_scale only applies to quantity = fission_heat.");

  for (unsigned int g = 0; g < _num_groups; ++g)
  {
    _group_vars.push_back(getVar("group_fluxes", g));
    if (&_group_vars[g]->sys() != &_sys)
      paramError("group_fluxes", "All group fluxes must be in the same system.");
  }
}

void
FissionWeightVector::initialSetup()
{
  // W is only valid while the cross sections do not depend on the solution. Catch materials on our
  // blocks that depend on a solver variable other than the group fluxes (which nuclear materials couple
  // only to compute reaction rates).
  const auto & materials = _fe_problem.getRegularMaterialsWarehouse()[Moose::BLOCK_MATERIAL_DATA];
  for (const auto block : blockIDs())
    if (materials.hasActiveBlockObjects(block, _tid))
      for (const auto & material : materials.getActiveBlockObjects(block, _tid))
        for (const auto * var : material->getMooseVariableDependencies())
          if (var->kind() == Moose::VAR_SOLVER &&
              std::find(_group_vars.begin(), _group_vars.end(), var) == _group_vars.end())
            mooseError("Material '",
                       material->name(),
                       "' couples the solver variable '",
                       var->name(),
                       "', so the cross sections may depend on the solution and the fission "
                       "integral is not W . u. Use ElmIntegTotFissHeatPostprocessor or "
                       "ElmIntegTotFissNtsPostprocessor instead.");
}

void
FissionWeightVector::initialize()
{
  if (_tid == 0)
    _weights.zero();
}

void
FissionWeightVector::execute()
{
  for (unsigned int g = 0; g < _num_groups; ++g)
  {
    const auto & phi = _group_vars[g]->phi();
    const auto & dofs = _group_vars[g]->dofIndices();
    _local_weights.resize(dofs.size());
    _local_weights.zero();
    for (unsigned int qp = 0; qp < _qrule->n_points(); ++qp)
    {
      const Real w = _JxW[qp] * _coord[qp] *
                     (_heat ? (*_fisse)[qp][g] * _xs[qp][g] * _nt_scale : _xs[qp][g]);
      for (unsigned int i = 0; i < dofs.size(); ++i)
        _local_weights(i) += w * phi[i][qp];
    }
    Threads::spin_mutex::scoped_lock lock(_mutex);
    _weights.add_vector(_local_weights, dofs);
  }
}

void
FissionWeightVector::finalize()
{
  _weights.close();
  _built = true;
}
