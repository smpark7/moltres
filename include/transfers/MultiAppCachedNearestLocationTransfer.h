#pragma once

#include "MultiAppGeneralFieldNearestLocationTransfer.h"

/**
 * Nearest-location transfer that caches, for every target DoF, which source location provides
 * its value. The first execution runs the regular nearest-location search once, with each source
 * value replaced by the global index of its source location, and records the index each target
 * DoF receives. Later executions only gather the source values and set the target DoFs by index,
 * skipping the point search and the point communication.
 *
 * Valid only while the source and target locations do not move (no adaptivity, no displacement
 * that changes between executions). The cache is rebuilt after a restart.
 */
class MultiAppCachedNearestLocationTransfer : public MultiAppGeneralFieldNearestLocationTransfer
{
public:
  static InputParameters validParams();

  MultiAppCachedNearestLocationTransfer(const InputParameters & parameters);

  void initialSetup() override;
  void execute() override;

protected:
  void prepareEvaluationOfInterpValues(const unsigned int var_index) override;

private:
  /// Run the nearest-location search once with source indices as values and record the mapping
  void buildCache();

  /// Gather the source values from all processes, ordered by global source index
  void gatherSourceValues(std::vector<Real> & values);

  /// Set the target DoFs from the gathered source values
  void setCachedValues(const std::vector<Real> & values);

  /// Whether the source values are replaced by their global source indices (cache building pass)
  bool _encode_source_index;

  /// Whether the target DoF to source index map has been built
  bool _cache_built;

  /// Number of source locations over all processes
  dof_id_type _n_global_sources;

  /// For each local target problem, the target DoFs that receive a source value
  std::vector<std::vector<dof_id_type>> _cached_dofs;

  /// For each local target problem, the global source index of each DoF in _cached_dofs
  std::vector<std::vector<dof_id_type>> _cached_indices;
};
