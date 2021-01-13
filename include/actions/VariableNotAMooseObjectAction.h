#pragma once

#include "Action.h"

class VariableNotAMooseObjectAction : public Action
{
public:
  VariableNotAMooseObjectAction(const InputParameters & params);

protected:
  /**
   * Get the block ids from the input parameters
   * @return A set of block ids defined in the input
   */
  std::set<SubdomainID> getSubdomainIDs();

  /**
   * Add a variable
   * @param var_name The variable name
   */
  void addVariable(const std::string & var_name);

  /**
   * Order of FE shape function
   */
  std::string _order;
};

template <>
InputParameters validParams<VariableNotAMooseObjectAction>();
