#pragma once

#include "ElementUserObject.h"
#include "ScalarTransportBase.h"

/**
 * Assembles the weight vector W of the fission heat or fission neutron production integral over its
 * blocks, so that the integral is W . u for the current solution u. This holds while the cross
 * sections depend only on auxiliary variables and the fluxes are not in exponential form. Rebuild W
 * whenever the cross sections change (e.g. on timestep_begin after a temperature transfer); then
 * WeightVectorDotProduct evaluates the integral without an element loop.
 */
class FissionWeightVector : public ElementUserObject, public ScalarTransportBase
{
public:
  static InputParameters validParams();

  FissionWeightVector(const InputParameters & parameters);

  virtual void initialSetup() override;
  virtual void initialize() override;
  virtual void execute() override;
  virtual void threadJoin(const UserObject &) override {}
  virtual void finalize() override;

  /// The weight vector, laid out like the solution of the flux variables' system
  const NumericVector<Number> & weights() const { return _weights; }
  /// The system the weight vector belongs to
  const SystemBase & system() const { return _sys; }
  /// Whether the weight vector has been assembled yet
  bool built() const { return _built; }

protected:
  /// The number of neutron energy groups
  const unsigned int _num_groups;
  /// Whether the weights are fission heat (fisse * fissxs) or fission neutron (nsf) weights
  const bool _heat;
  /// Scaling of the neutron fluxes; multiplies the heat weights only, as in
  /// ElmIntegTotFissHeatPostprocessor
  const Real _nt_scale;
  /// Group flux variables
  std::vector<const MooseVariable *> _group_vars;
  /// fissxs or nsf
  const MaterialProperty<std::vector<Real>> & _xs;
  /// fisse, for the heat weights
  const MaterialProperty<std::vector<Real>> * const _fisse;
  /// Solver system of the group fluxes
  SystemBase & _sys;
  /// Weight vector, stored in _sys so it follows its dof layout
  NumericVector<Number> & _weights;
  /// Whether the weight vector has been assembled yet
  bool _built;
  /// Element weights of one group
  DenseVector<Number> _local_weights;
  /// Serializes the additions to _weights across threads
  static Threads::spin_mutex _mutex;
};
