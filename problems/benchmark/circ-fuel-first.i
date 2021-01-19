density = .002  # kg cm-3
c_p = 3075      # J cm-3 K-1
k = .005        # W cm-1 K-1
gamma = 1       # W cm-3 K-1, Volumetric heat transfer coefficient
tau = .2        # SUPG stabilization factor

[GlobalParams]
  num_groups = 6
  num_precursor_groups = 8
  use_exp_form = false
  group_fluxes = 'group1 group2 group3 group4 group5 group6'
  pre_concs = 'pre1 pre2 pre3 pre4 pre5 pre6 pre7 pre8'
  temperature = 900
  sss2_input = true
  account_delayed = true
[../]

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 100
  ny = 100
  xmin = 0
  xmax = 200
  ymin = 0
  ymax = 200
[]

[Problem]
  type = FEProblem
[]

[Nt]
  var_name_base = group
  vacuum_boundaries = 'bottom left right top'
  create_temperature_var = false
  eigen = true
[]

[Precursors]
  [./pres]
    var_name_base = pre
    outlet_boundaries = ''
    constant_velocity_values = false
    uvel = ux
    vvel = uy
    nt_exp_form = false
    family = LAGRANGE
    order = FIRST
    transient = false
    add_art_diff = true
    prec_diffusivity = prec_diff
  [../]
[]

[AuxVariables]
  [./ux]
    family = LAGRANGE
    order = FIRST
  [../]
  [./uy]
    family = LAGRANGE
    order = FIRST
  [../]
[]

[UserObjects]
  [./velocities]
    type = SolutionUserObject
    mesh = './vel-field-stabilized_exodus.e'
    system_variables = 'ux uy'
    timestep = LATEST
    execute_on = INITIAL
  [../]
[]

[AuxKernels]
  [./ux]
    type = SolutionAux
    variable = ux
    from_variable = ux
    solution = velocities
  [../]
  [./uy]
    type = SolutionAux
    variable = uy
    from_variable = uy
    solution = velocities
  [../]
[]

[Materials]
  [./fuel]
    type = GenericMoltresMaterial
    property_tables_root = './xs/benchmark_'
    interp_type = 'linear'
    prop_names = 'prec_diff'
    prop_values = '1.25e-06'
  [../]
[]

[Executioner]
  type = InversePowerMethod
  max_power_iterations = 50

  # fission power normalization
  normalization = 'powernorm'
  normal_factor = 1e7           # Watts, 1e9 / 100

  xdiff = 'group1diff'
  bx_norm = 'bnorm'
  k0 = 1.00400
  pfactor = 1e-2
  l_max_its = 100

  # solve_type = 'PJFNK'
  solve_type = 'NEWTON'
  petsc_options = '-snes_converged_reason -ksp_converged_reason -snes_linesearch_monitor'
#  petsc_options_iname = '-pc_type -pc_factor_shift_type -pc_factor_mat_solver_package'
#  petsc_options_value = 'lu       NONZERO               superlu_dist'
  petsc_options_iname = '-pc_type -sub_pc_type'
  petsc_options_value = 'asm      lu'
[]

[Preconditioning]
  [./SMP]
    type = SMP
    full = true
  [../]
[]

[Postprocessors]
  [./bnorm]
    type = ElmIntegTotFissNtsPostprocessor
    execute_on = linear
  [../]
  [./tot_fissions]
    type = ElmIntegTotFissPostprocessor
    execute_on = linear
  [../]
  [./powernorm]
    type = ElmIntegTotFissHeatPostprocessor
    execute_on = linear
  [../]
  [./group1norm]
    type = ElementIntegralVariablePostprocessor
    variable = group1
    execute_on = linear
  [../]
  [./group1max]
    type = NodalMaxValue
    variable = group1
    execute_on = timestep_end
  [../]
  [./group1diff]
    type = ElementL2Diff
    variable = group1
    execute_on = 'linear timestep_end'
    use_displaced_mesh = false
  [../]
  [./group2norm]
    type = ElementIntegralVariablePostprocessor
    variable = group2
    execute_on = linear
  [../]
  [./group2max]
    type = NodalMaxValue
    variable = group2
    execute_on = timestep_end
  [../]
  [./group2diff]
    type = ElementL2Diff
    variable = group2
    execute_on = 'linear timestep_end'
    use_displaced_mesh = false
  [../]
[]

[VectorPostprocessors]
  [./aa]
    type = LineValueSampler
    variable = 'pre1 pre2 pre3 pre4 pre5 pre6 pre7 pre8'
    start_point = '0 100 0'
    end_point = '200 100 0'
    num_points = 201
    sort_by = x
    execute_on = FINAL
  [../]
  [./bb]
    type = LineValueSampler
    variable = 'pre1 pre2 pre3 pre4 pre5 pre6 pre7 pre8'
    start_point = '100 0 0'
    end_point = '100 200 0'
    num_points = 201
    sort_by = y
    execute_on = FINAL
  [../]
[]

[Outputs]
  perf_graph = true
  print_linear_residuals = true
  csv = true
  [./out]
    type = Exodus
  [../]
[]

[Debug]
  show_var_residual_norms = true
[]
