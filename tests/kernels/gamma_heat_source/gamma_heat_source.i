# Gamma heating source with a space- and time-dependent gamma function: T_t - div grad T = P * gamma(x, y, t)
[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 4
    ny = 4
    elem_type = QUAD9
  []
[]

[Variables]
  [T]
    order = SECOND
  []
[]

[Functions]
  [gamma_fn]
    type = ParsedFunction
    expression = '(1 + x * y) * (1 + t)'
  []
[]

[Kernels]
  [time]
    type = TimeDerivative
    variable = T
  []
  [diffusion]
    type = Diffusion
    variable = T
  []
  [gamma_heat]
    type = GammaHeatSource
    variable = T
    average_fission_heat = average_heat
    gamma = gamma_fn
  []
[]

[BCs]
  [sides]
    type = DirichletBC
    variable = T
    boundary = 'left right'
    value = 0
  []
[]

[Postprocessors]
  [average_heat]
    type = ConstantPostprocessor
    value = 2.5
  []
  [T_integral]
    type = ElementIntegralVariablePostprocessor
    variable = T
  []
  [T_max]
    type = ElementExtremeValue
    variable = T
  []
[]

[Preconditioning]
  [smp]
    type = SMP
    full = true
  []
[]

[Executioner]
  type = Transient
  num_steps = 2
  dt = 0.5
  solve_type = NEWTON
[]

[Outputs]
  csv = true
[]
