[GlobalParams]
  num_groups = 2
  num_precursor_groups = 6
  group_fluxes = 'group1 group2'
  pre_concs = 'pre1 pre2 pre3 pre4 pre5 pre6'
  temperature = 922
  transient = true
  integrate_p_by_parts = true
  account_delayed = true
[]

[Mesh]
  [./mesh]
    type = FileMeshGenerator
    file = 'msre-steady-state-2nd-flow_exodus.e'
    use_for_exodus_restart = true
  []
[]

[Problem]
  type = FEProblem
  coord_type = RZ
  kernel_coverage_check = false
[]

[Precursors]
  [./pres]
    var_name_base = pre
    outlet_boundaries = 'fuel_top'
    constant_velocity_values = false
    u_func = 0
    v_func = vel_func
    w_func = 0
    nt_exp_form = false
    family = MONOMIAL
    order = SECOND
    loop_precursors = true
    multi_app = loopApp
    is_loopapp = false
    inlet_boundaries = 'fuel_bottom'
    block = 0
    init_from_file = false
  [../]
[]

[AuxVariables]
  [./group1]
    family = LAGRANGE
    order = FIRST
    initial_from_file_var = group1
    initial_from_file_timestep = LATEST
  []
  [./group2]
    family = LAGRANGE
    order = FIRST
    initial_from_file_var = group2
    initial_from_file_timestep = LATEST
  []
  [./group1_source]
    family = LAGRANGE
    order = FIRST
    initial_from_file_var = group1_source
    initial_from_file_timestep = LATEST
  []
  [./group2_source]
    family = LAGRANGE
    order = FIRST
    initial_from_file_var = group2_source
    initial_from_file_timestep = LATEST
  []
[]

[Kernels]
  [./pre1_advection]
    type = CtrlConservativeAdvection
    variable = pre1
    block = 0
    u_val = 0
    v_val = 21.45
    w_val = 0
  [../]
  [./pre2_advection]
    type = CtrlConservativeAdvection
    variable = pre2
    block = 0
    u_val = 0
    v_val = 21.45
    w_val = 0
  [../]
  [./pre3_advection]
    type = CtrlConservativeAdvection
    variable = pre3
    block = 0
    u_val = 0
    v_val = 21.45
    w_val = 0
  [../]
  [./pre4_advection]
    type = CtrlConservativeAdvection
    variable = pre4
    block = 0
    u_val = 0
    v_val = 21.45
    w_val = 0
  [../]
  [./pre5_advection]
    type = CtrlConservativeAdvection
    variable = pre5
    block = 0
    u_val = 0
    v_val = 21.45
    w_val = 0
  [../]
  [./pre6_advection]
    type = CtrlConservativeAdvection
    variable = pre6
    block = 0
    u_val = 0
    v_val = 21.45
    w_val = 0
  [../]
[]

[AuxKernels]
  [./group1_normalization]
    type = NormalizationAux
    variable = group1
    source_variable = group1_source
    normalization = bnorm
    execute_on = linear
  []
  [./group2_normalization]
    type = NormalizationAux
    variable = group2
    source_variable = group2_source
    normalization = bnorm
    execute_on = linear
  []
[]

[UserObjects]
  [./soln]
    type = SolutionUserObject
    mesh = msre-steady-state-2nd-flow_exodus.e
    execute_on = INITIAL
    system_variables = 'pre1 pre2 pre3 pre4 pre5 pre6'
    timestep = LATEST
  []
[]

[Functions]
  [./vel_func]
    type = ParsedFunction
    value = 'if(t<0.0, 21.45, if(t<20.0, 21.45 * 0.01 * (100 * ( 0.08567004393586444 / (1 + exp( 0.18270364351392387 * (t - 11.348385006979589 ))) + 0.11212706244321342 / (1 + exp( 0.6423926745267268 * (t - 8.720664295533949 ))) + 0.48132650370444385 / (1 + exp( 2.1187238480013177 * (t - 2.746163100638864 ))) + 0.21846374585678544 / (1 + exp( 3.1513341812734037 * (t - 5.142977826914452 )))) + 11.385268805267097 / (1 + exp( 7.028373826005297 * (t - 1.1677113693268026 ))) - 1.322249566482958 / (1 + exp(- 15.020553937112997 * (t - 4.482602484800283 )))), 1e-14))'
  []
  [./dt_func]
    type = ParsedFunction
    value = 'if(t<0.0, if(t<-200.0, 1e-6, 20), 0.1)'
  []
  [./pre1_func]
    type = SolutionFunction
    solution = soln
    from_variable = pre1
    execute_on = INITIAL
  []
  [./pre2_func]
    type = SolutionFunction
    solution = soln
    from_variable = pre2
    execute_on = INITIAL
  []
  [./pre3_func]
    type = SolutionFunction
    solution = soln
    from_variable = pre3
    execute_on = INITIAL
  []
  [./pre4_func]
    type = SolutionFunction
    solution = soln
    from_variable = pre4
    execute_on = INITIAL
  []
  [./pre5_func]
    type = SolutionFunction
    solution = soln
    from_variable = pre5
    execute_on = INITIAL
  []
  [./pre6_func]
    type = SolutionFunction
    solution = soln
    from_variable = pre6
    execute_on = INITIAL
  []
[]

[Controls]
  [./func_control]
    type = RealFunctionControl
    parameter = '*/*/v_val'
    function = 'vel_func'
    execute_on = 'initial timestep_begin'
  []
[]

[ICs]
  [./pre1_ic]
    type = FunctionIC
    variable = pre1
    function = pre1_func
  []
  [./pre2_ic]
    type = FunctionIC
    variable = pre2
    function = pre2_func
  []
  [./pre3_ic]
    type = FunctionIC
    variable = pre3
    function = pre3_func
  []
  [./pre4_ic]
    type = FunctionIC
    variable = pre4
    function = pre4_func
  []
  [./pre5_ic]
    type = FunctionIC
    variable = pre5
    function = pre5_func
  []
  [./pre6_ic]
    type = FunctionIC
    variable = pre6
    function = pre6_func
  []
[]

[Materials]
  [./fuel_nts]
    type = MoltresJsonMaterial
    block = '0'
    base_file = 'xsdata.json'
    material_key = 'fuel'
    interp_type = 'NONE'
    prop_names = ''
    prop_values = ''
  [../]
  [./mod]
    type = MoltresJsonMaterial
    block = '1'
    base_file = 'xsdata.json'
    material_key = 'mod'
    interp_type = 'NONE'
    prop_names = ''
    prop_values = ''
  [../]
[]

[Executioner]
  type = Transient
  scheme = bdf2
  start_time = -200.000002
  end_time = 70

  solve_type = 'NEWTON'
  petsc_options = '-snes_converged_reason -ksp_converged_reason -snes_linesearch_monitor'
  petsc_options_iname = '-pc_type -sub_pc_type -ksp_gmres_restart -pc_asm_overlap -sub_pc_factor_shift_type'
  petsc_options_value = 'asm      lu           200                1               NONZERO'
#  petsc_options_iname = '-pc_type -pc_factor_shift_type -pc_factor_mat_solver_type'
#  petsc_options_value = 'lu       NONZERO               superlu_dist'

  nl_abs_tol = 1e-9
  nl_forced_its = 1
  l_tol = 1e-5
  line_search = none
  automatic_scaling = true
  compute_scaling_once = false
  resid_vs_jac_scaling_param = 0.1

  auto_advance = true
  fixed_point_abs_tol = 1e-7
  fixed_point_max_its = 5

  [./TimeStepper]
    type = FunctionDT
    function = dt_func
  [../]
[]

[Preconditioning]
  [./SMP]
    type = SMP
    full = true
  [../]
[]

[Postprocessors]
  [./bnorm]
    type = Receiver
    default = 1
  []
[]

[MultiApps]
  [./loopApp]
    type = TransientMultiApp
    app_type = MoltresApp
    execute_on = timestep_begin
    positions = '100 0 0'
    input_files = 'msre-coastdown-2nd-loop.i'
  []
  [./ntsApp]
    type = FullSolveMultiApp
    app_type = MoltresApp
    execute_on = timestep_end
    positions = '0 0 0'
    input_files = 'msre-transient-nts-2nd.i'
  []
[]

[Transfers]
  [./to_nts_pre]
    type = MultiAppCopyTransfer
    direction = to_multiapp
    multi_app = ntsApp
    source_variable = 'pre1 pre2 pre3 pre4 pre5 pre6'
    variable = 'pre1 pre2 pre3 pre4 pre5 pre6'
  []
  [./from_nts_flux]
    type = MultiAppCopyTransfer
    direction = from_multiapp
    multi_app = ntsApp
    source_variable = 'group1 group2'
    variable = 'group1_source group2_source'
  []
  [./from_nts_k_eff]
    type = MultiAppPostprocessorTransfer
    direction = from_multiapp
    multi_app = ntsApp
    from_postprocessor = bnorm
    to_postprocessor = bnorm
    reduction_type = average
  []
[]

[Outputs]
  perf_graph = true
  print_linear_residuals = true
  [./exodus]
    type = Exodus
  []
  [./csv]
    type = CSV
  []
[]

[Debug]
  show_var_residual_norms = true
[]
