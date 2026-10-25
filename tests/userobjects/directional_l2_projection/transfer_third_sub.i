# 1-D channel receiving the third-order directional L2 projection of transfer_third.i in a LAGRANGE
# THIRD variable.

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 1
    nx = 4
    xmax = 1.6
    elem_type = EDGE4
  []
[]

[Variables]
  [dummy]
    family = LAGRANGE
    order = THIRD
  []
[]

[Kernels]
  [diff]
    type = Diffusion
    variable = dummy
  []
[]

[Functions]
  [average]
    type = ParsedFunction
    expression = '1.0133333333333334 + 2*x - 0.5*x*x + 0.25*x*x*x'
  []
[]

[AuxVariables]
  [received]
    family = LAGRANGE
    order = THIRD
  []
[]

[Postprocessors]
  [err]
    type = ElementL2Error
    variable = received
    function = average
  []
  [integral]
    type = ElementIntegralVariablePostprocessor
    variable = received
  []
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Steady
[]

[Outputs]
  csv = true
[]
