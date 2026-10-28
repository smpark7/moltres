#pragma once

#include "NuclearMaterial.h"
#include "nlohmann/json.h"

class MoltresJsonMaterial : public NuclearMaterial
{
public:
  MoltresJsonMaterial(const InputParameters & parameters);

  static InputParameters validParams();

protected:
  void Construct(nlohmann::json xs_root);
  /// Reads a table with several state variables ('axes' and 'branches' in the material entry)
  void constructMultiD(const nlohmann::json & material);
  /**
   * Appends the group constants of each branch to _xsec_map, in branch order.
   * @param branches JSON object of each branch
   * @param labels Name of each branch, used in messages
   */
  void readGroupConstants(const std::vector<const nlohmann::json *> & branches,
                          const std::vector<std::string> & labels);
  /// Errors if a least-squares fit of the requested order is underdetermined on some axis
  void checkLsqKnots(const XsAxes & axes, const std::vector<std::string> & axis_names);
  virtual void computeQpProperties() override;
  virtual void computeXsState() override;

  // Vector of group constants to be loaded
  std::vector<std::string> _group_consts;

  // Material associated with the group constants to be loaded
  std::string _material_key;

  /// Functor of each axis of a multi-dimensional table; null for the axis set by 'temperature'
  std::vector<const Moose::Functor<Real> *> _axis_functors;
};
