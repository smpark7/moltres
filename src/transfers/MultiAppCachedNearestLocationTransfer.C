#include "MultiAppCachedNearestLocationTransfer.h"

#include "FEProblemBase.h"
#include "MooseVariableFieldBase.h"

#include "libmesh/dof_map.h"
#include "libmesh/numeric_vector.h"
#include "libmesh/system.h"

#include <cmath>
#include <limits>

registerMooseObject("MoltresApp", MultiAppCachedNearestLocationTransfer);

InputParameters
MultiAppCachedNearestLocationTransfer::validParams()
{
  InputParameters params = MultiAppGeneralFieldNearestLocationTransfer::validParams();
  params.addClassDescription(
      "Nearest-location transfer that searches once, caches the source location of every target "
      "degree of freedom, and afterwards only communicates the source values. Valid only for "
      "meshes whose node positions do not change between executions.");
  return params;
}

MultiAppCachedNearestLocationTransfer::MultiAppCachedNearestLocationTransfer(
    const InputParameters & parameters)
  : MultiAppGeneralFieldNearestLocationTransfer(parameters),
    _encode_source_index(false),
    _cache_built(false),
    _n_global_sources(0)
{
  if (_num_nearest_points != 1)
    paramError("num_nearest_points", "Only a single nearest point is supported");
  if (_from_var_names.size() != 1 || _to_var_names.size() != 1)
    paramError("variable", "Only a single variable per transfer is supported");
  if (!_from_var_components.empty() || !_to_var_components.empty())
    paramError("source_variable_components", "Array variable components are not supported");
  if (_post_transfer_extrapolation != "none")
    paramError("post_transfer_extrapolation", "Only 'none' is supported");
  // Missed target points must keep their values, which the cache build relies on to tell them
  // apart from the received source indices
  if (!GeneralFieldTransfer::isOutOfMeshValue(getParam<Real>("extrapolation_constant")))
    mooseError("Only transfers leaving the target values unchanged at missed points are supported");
  if (_preserve_transfer)
    paramError("from_postprocessors_to_be_preserved",
               "Conservative adjustment is not supported: it would act on the source indices "
               "while the cache is built");
}

void
MultiAppCachedNearestLocationTransfer::initialSetup()
{
  MultiAppGeneralFieldNearestLocationTransfer::initialSetup();

  // Target values are cached per DoF, so each targeted DoF must take its value from a single
  // location (nodal or constant elemental variables, not projected higher-order elemental ones)
  if (_to_problems.size())
  {
    const auto * const to_var = getToVariable(0);
    if (!to_var->isNodal() && to_var->feType().order > CONSTANT)
      paramError("variable", "Only nodal or constant elemental target variables are supported");
  }
}

void
MultiAppCachedNearestLocationTransfer::execute()
{
  if (!_cache_built)
    buildCache();

  TIME_SECTION("MultiAppCachedNearestLocationTransfer::execute()_" + name(),
               5,
               "Cached transfer execution " + name());
  getAppInfo();

  std::vector<Real> values;
  gatherSourceValues(values);
  setCachedValues(values);

  postExecute();
}

void
MultiAppCachedNearestLocationTransfer::prepareEvaluationOfInterpValues(const unsigned int var_index)
{
  MultiAppGeneralFieldNearestLocationTransfer::prepareEvaluationOfInterpValues(var_index);
  if (!_encode_source_index)
    return;

  // Global index of a source location = offset of this process + position in the local sources,
  // in the order they are gathered by gatherSourceValues
  dof_id_type n_local = 0;
  for (const auto & values : _local_values)
    n_local += values.size();
  std::vector<dof_id_type> n_per_proc;
  _communicator.allgather(n_local, n_per_proc);

  dof_id_type index = 0;
  for (const auto pid : make_range(processor_id()))
    index += n_per_proc[pid];
  _n_global_sources = 0;
  for (const auto n : n_per_proc)
    _n_global_sources += n;
  if (_n_global_sources > (dof_id_type(1) << std::numeric_limits<Real>::digits))
    mooseError("Too many source locations to encode as exact floating point values");

  for (auto & values : _local_values)
    for (auto & value : values)
      value = index++;
}

void
MultiAppCachedNearestLocationTransfer::buildCache()
{
  TIME_SECTION("MultiAppCachedNearestLocationTransfer::buildCache()_" + name(),
               5,
               "Building transfer cache " + name());

  const auto & var_name = getToVarName(0);

  // Mark the local target DoFs as unset, keeping their values to restore the DoFs the transfer
  // does not reach
  std::vector<std::vector<dof_id_type>> dofs(_to_problems.size());
  std::vector<std::vector<Real>> saved(_to_problems.size());
  for (const auto i_to : index_range(_to_problems))
  {
    auto & to_sys = *find_sys(getEquationSystem(*_to_problems[i_to], false), var_name);
    to_sys.get_dof_map().local_variable_indices(
        dofs[i_to], to_sys.get_mesh(), to_sys.variable_number(var_name));
    to_sys.solution->get(dofs[i_to], saved[i_to]);
    to_sys.solution->insert(
        std::vector<Real>(dofs[i_to].size(), std::numeric_limits<Real>::quiet_NaN()), dofs[i_to]);
    to_sys.solution->close();
  }

  // Regular nearest-location transfer of the source indices
  _encode_source_index = true;
  MultiAppGeneralFieldNearestLocationTransfer::execute();
  _encode_source_index = false;

  // Record the source index each DoF received and restore the DoFs that received nothing
  _cached_dofs.assign(_to_problems.size(), {});
  _cached_indices.assign(_to_problems.size(), {});
  for (const auto i_to : index_range(_to_problems))
  {
    auto & to_sys = *find_sys(getEquationSystem(*_to_problems[i_to], false), var_name);
    std::vector<Real> received;
    to_sys.solution->get(dofs[i_to], received);

    std::vector<dof_id_type> unset_dofs;
    std::vector<Real> unset_values;
    for (const auto i : index_range(dofs[i_to]))
    {
      const auto dof = dofs[i_to][i];
      const Real value = received[i];
      if (std::isnan(value))
      {
        unset_dofs.push_back(dof);
        unset_values.push_back(saved[i_to][i]);
        continue;
      }
      const auto index = static_cast<dof_id_type>(value);
      if (value < 0 || static_cast<Real>(index) != value || index >= _n_global_sources)
        mooseError("Unexpected value ", value, " while building the transfer cache");
      _cached_dofs[i_to].push_back(dof);
      _cached_indices[i_to].push_back(index);
    }
    // Closed when execute() sets the cached values
    to_sys.solution->insert(unset_values, unset_dofs);
  }
  _cache_built = true;
}

void
MultiAppCachedNearestLocationTransfer::gatherSourceValues(std::vector<Real> & values)
{
  MultiAppGeneralFieldNearestLocationTransfer::prepareEvaluationOfInterpValues(0);
  values.clear();
  for (const auto & local_values : _local_values)
    values.insert(values.end(), local_values.begin(), local_values.end());
  _communicator.allgather(values, false);

  if (values.size() != _n_global_sources)
    mooseError("The number of source locations changed from ",
               _n_global_sources,
               " to ",
               values.size(),
               " since the transfer cache was built. The source mesh must not change.");
}

void
MultiAppCachedNearestLocationTransfer::setCachedValues(const std::vector<Real> & values)
{
  const auto & var_name = getToVarName(0);
  for (const auto i_to : index_range(_to_problems))
  {
    auto & to_sys = *find_sys(getEquationSystem(*_to_problems[i_to], false), var_name);
    std::vector<Real> dof_values;
    dof_values.reserve(_cached_indices[i_to].size());
    for (const auto index : _cached_indices[i_to])
      dof_values.push_back(values[index]);
    to_sys.solution->insert(dof_values, _cached_dofs[i_to]);
    to_sys.solution->close();
    to_sys.update();
  }
}
