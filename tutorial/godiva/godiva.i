[GlobalParams]
  num_groups = 6
  num_precursor_groups = 8
  use_exp_form = false
  group_fluxes = 'group1 group2 group3 group4 group5 group6'
  pre_concs = 'pre1 pre2 pre3 pre4 pre5 pre6 pre7 pre8'
  temperature = 1200
  sss2_input = true
  account_delayed = true
[../]

[Mesh]
  [./generated_mesh]
    type = GeneratedMeshGenerator
    dim = 1
    nx = 1000
    xmin = 0
    xmax = 8.7407
  []
[]

[Problem]
  type = FEProblem
  coord_type = RSPHERICAL
[]

[Nt]
  var_name_base = group
  vacuum_boundaries = 'right'
  create_temperature_var = false
  eigen = true
  transient = false
  scaling = 1e3
[]

[Precursors]
  [./pres]
    var_name_base = pre
    outlet_boundaries = ''
    constant_velocity_values = true
    u_def = 0
    v_def = 0
    w_def = 0
    nt_exp_form = false
    family = MONOMIAL
    order = CONSTANT
    loop_precursors = false
    transient = false
    scaling = 1e3
    eigen = true
  [../]
[]

[Materials]
  [./fuel]
    type = MoltresJsonMaterial
    block = 0
    base_file = 'godiva_serpent.json'
#    base_file = 'godiva_openmc.json'
    material_key = 'fuel'
#    material_key = 'fuel_mat'
    interp_type = 'SPLINE'
    prop_names = ''
    prop_values = ''
  []
[]

[Executioner]
  type = InversePowerMethod
  max_power_iterations = 50

  normalization = 'powernorm'
  normal_factor = 1e3

  xdiff = 'group1diff'
  bx_norm = 'bnorm'
  k0 = 1.00400
  l_max_its = 100
  eig_check_tol = 1e-7

  solve_type = 'NEWTON'
  petsc_options = '-snes_converged_reason -ksp_converged_reason -snes_linesearch_monitor'
  petsc_options_iname = '-pc_type -pc_factor_shift_type -pc_factor_mat_solver_package'
  petsc_options_value = 'lu       NONZERO               superlu_dist'
  line_search = none
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

[Outputs]
  perf_graph = true
  print_linear_residuals = true
  [./exodus]
    type = Exodus
  [../]
  [./csv]
    type = CSV
  [../]
[]

[Debug]
  show_var_residual_norms = true
[]
