tmin = 450
tmax = 1350

# Samples the group constants and their temperature derivatives along a temperature ramp that
# extends below and above the tabulated range. The material and the ramp are set on the command
# line (see the tests file), so that the same input serves both material classes.

[GlobalParams]
  num_groups = 2
  num_precursor_groups = 6
  temperature = temp
[]

[Mesh]
  [mesh]
    type = GeneratedMeshGenerator
    dim = 1
    nx = 90
  []
[]

# A trivial nonlinear solve, so that the derivative properties are evaluated during Jacobian assembly
[Variables]
  [u]
  []
[]

[Kernels]
  [diff]
    type = Diffusion
    variable = u
  []
[]

[BCs]
  [left]
    type = DirichletBC
    variable = u
    boundary = left
    value = 0
  []
  [right]
    type = DirichletBC
    variable = u
    boundary = right
    value = 1
  []
[]

[AuxVariables]
  [temp]
  []
  [nsf_0]
    family = MONOMIAL
    order = CONSTANT
  []
  [fisse_1]
    family = MONOMIAL
    order = CONSTANT
  []
  [gtransfxs_1]
    family = MONOMIAL
    order = CONSTANT
  []
  [beta]
    family = MONOMIAL
    order = CONSTANT
  []
  [decay_constant_5]
    family = MONOMIAL
    order = CONSTANT
  []
  [d_nsf_d_temp_0]
    family = MONOMIAL
    order = CONSTANT
  []
  [d_fisse_d_temp_1]
    family = MONOMIAL
    order = CONSTANT
  []
  [d_gtransfxs_d_temp_1]
    family = MONOMIAL
    order = CONSTANT
  []
  [d_beta_d_temp]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[ICs]
  [temp]
    type = FunctionIC
    variable = temp
    function = '${tmin} + (${tmax} - ${tmin}) * x'
  []
[]

[AuxKernels]
  [nsf_0]
    type = MaterialStdVectorAux
    variable = nsf_0
    property = nsf
    index = 0
  []
  [fisse_1]
    type = MaterialStdVectorAux
    variable = fisse_1
    property = fisse
    index = 1
  []
  [gtransfxs_1]
    type = MaterialStdVectorAux
    variable = gtransfxs_1
    property = gtransfxs
    index = 1
  []
  [beta]
    type = MaterialRealAux
    variable = beta
    property = beta
  []
  [decay_constant_5]
    type = MaterialStdVectorAux
    variable = decay_constant_5
    property = decay_constant
    index = 5
  []
  # Derivatives are only computed during Jacobian assembly
  [d_nsf_d_temp_0]
    type = MaterialStdVectorAux
    variable = d_nsf_d_temp_0
    property = d_nsf_d_temp
    index = 0
    execute_on = nonlinear
  []
  [d_fisse_d_temp_1]
    type = MaterialStdVectorAux
    variable = d_fisse_d_temp_1
    property = d_fisse_d_temp
    index = 1
    execute_on = nonlinear
  []
  [d_gtransfxs_d_temp_1]
    type = MaterialStdVectorAux
    variable = d_gtransfxs_d_temp_1
    property = d_gtransfxs_d_temp
    index = 1
    execute_on = nonlinear
  []
  [d_beta_d_temp]
    type = MaterialRealAux
    variable = d_beta_d_temp
    property = d_beta_d_temp
    execute_on = nonlinear
  []
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
[]

[Outputs]
  exodus = true
[]
