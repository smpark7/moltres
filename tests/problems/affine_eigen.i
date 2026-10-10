[GlobalParams]
  num_groups = 8
  num_precursor_groups = 6
  group_fluxes = 'group1 group2 group3 group4 group5 group6 group7 group8'
  pre_concs = ''
  temperature = 900
  sss2_input = true
  account_delayed = true
  use_exp_form = false
  delayed_neutron_source = dns
[]

[Problem]
  type = AffineEigenProblem
  bx_norm = bnorm
[]

[Mesh]
  [mesh]
    type = CartesianMeshGenerator
    dim = 3
    dx = 10
    dy = 10
    dz = 100
    ix = 2
    iy = 2
    iz = 40
  []
[]

[Nt]
  family = LAGRANGE
  order = FIRST
  var_name_base = group
  create_temperature_var = false
  eigen = true
[]

# The delayed neutron source is a solution-independent AuxVariable, which stock
# PJFNKMO with constant_matrices = true drops from the residual.
[AuxVariables]
  [dns]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[AuxKernels]
  [dns]
    type = FunctionAux
    variable = dns
    function = '1e-4*sin(pi*z/100)'
    execute_on = initial
  []
[]

[Materials]
  [fuel]
    type = MoltresJsonMaterial
    base_file = '../../property_file_dir/sn-test/absorber-air-lattice-ref.json'
    material_key = 'fuel'
    interp_type = 'none'
  []
[]

[Executioner]
  type = Eigenvalue
  free_power_iterations = 2
  nl_abs_tol = 1e-10
  nl_rel_tol = 1e-10
  solve_type = PJFNKMO
  constant_matrices = true
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
  line_search = none
[]

[Postprocessors]
  [bnorm]
    type = ElmIntegTotFissNtsPostprocessor
    execute_on = 'initial linear timestep_end'
    account_delayed = false
  []
  [dsrc]
    type = ElementIntegralVariablePostprocessor
    variable = dns
  []
  [eigenvalue]
    type = VectorPostprocessorComponent
    vectorpostprocessor = eigenvalues
    vector_name = eigen_values_real
    index = 0
  []
[]

[VectorPostprocessors]
  [eigenvalues]
    type = Eigenvalues
    inverse_eigenvalue = true
    outputs = none
  []
[]

[Outputs]
  csv = true
[]
