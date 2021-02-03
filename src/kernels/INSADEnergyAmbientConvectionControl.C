//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "INSADEnergyAmbientConvectionControl.h"
#include "INSADObjectTracker.h"
#include "FEProblemBase.h"

registerMooseObject("NavierStokesApp", INSADEnergyAmbientConvectionControl);

InputParameters
INSADEnergyAmbientConvectionControl::validParams()
{
  InputParameters params = ADKernelValue::validParams();
  params.addClassDescription(
      "Computes a controllable heat source/sink due to convection from ambient surroundings.");
  params.addRequiredParam<Real>("alpha",
                                "The heat transfer coefficient from the ambient surroundings");
  params.declareControllable("alpha"),
  params.addRequiredParam<Real>("T_ambient", "The ambient temperature");
  return params;
}

INSADEnergyAmbientConvectionControl::INSADEnergyAmbientConvectionControl(const InputParameters & parameters)
  : ADKernelValue(parameters),
    _heat_alpha(getParam<Real>("alpha")),
    _temperature_ambient_convection_strong_residual(
        getADMaterialProperty<Real>("temperature_ambient_convection_strong_residual"))
{
  // Bypass the UserObjectInterface method because it requires a UserObjectName param which we
  // don't need
  auto & obj_tracker = const_cast<INSADObjectTracker &>(
      _fe_problem.getUserObject<INSADObjectTracker>("ins_ad_object_tracker"));
  for (const auto block_id : blockIDs())
  {
    obj_tracker.set("has_ambient_convection", true, block_id);
    obj_tracker.set("ambient_convection_alpha", _heat_alpha, block_id);
    obj_tracker.set("ambient_temperature", getParam<Real>("T_ambient"), block_id);
  }
}

ADReal
INSADEnergyAmbientConvectionControl::precomputeQpResidual()
{
  return _temperature_ambient_convection_strong_residual[_qp];
}
