# 1-D channel receiving the directional L2 projection of transfer.i in a LAGRANGE SECOND variable.

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 1
    nx = 4
    xmax = 1.6
    elem_type = EDGE3
  []
[]

[Variables]
  [dummy]
    family = LAGRANGE
    order = SECOND
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
    expression = '1.0133333333333334 + 2*x - 0.5*x*x'
  []
[]

[AuxVariables]
  [received]
    family = LAGRANGE
    order = SECOND
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
