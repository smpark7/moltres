[GlobalParams]
  num_groups = 0
  num_precursor_groups = 6
  temperature = 922
  group_fluxes = ''
  sss2_input = false
[]

[Mesh]
  [./mesh]
    type = FileMeshGenerator
    file = 'msre-steady-state-2nd-flow_out_loopApp0_out.e'
  []
[../]

[Precursors]
  [./core]
    var_name_base = pre
    outlet_boundaries = 'right'
    constant_velocity_values = false
    u_func = vel_func
    v_func = 0
    w_func = 0
    nt_exp_form = false
    family = MONOMIAL
    order = SECOND
    loop_precursors = true
    multi_app = loopApp
    is_loopapp = true
    inlet_boundaries = 'left'
  [../]
[]

[Kernels]
  [./pre1_advection]
    type = CtrlConservativeAdvection
    variable = pre1
    u_val = 21.45
    v_val = 0
    w_val = 0
  []
  [./pre2_advection]
    type = CtrlConservativeAdvection
    variable = pre2
    u_val = 21.45
    v_val = 0
    w_val = 0
  []
  [./pre3_advection]
    type = CtrlConservativeAdvection
    variable = pre3
    u_val = 21.45
    v_val = 0
    w_val = 0
  []
  [./pre4_advection]
    type = CtrlConservativeAdvection
    variable = pre4
    u_val = 21.45
    v_val = 0
    w_val = 0
  []
  [./pre5_advection]
    type = CtrlConservativeAdvection
    variable = pre5
    u_val = 21.45
    v_val = 0
    w_val = 0
  []
  [./pre6_advection]
    type = CtrlConservativeAdvection
    variable = pre6
    u_val = 21.45
    v_val = 0
    w_val = 0
  []
[]

[UserObjects]
  [./soln]
    type = SolutionUserObject
    mesh = msre-steady-state-2nd-flow_out_loopApp0_out.e
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
    parameter = '*/*/u_val'
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
[]

[Executioner]
  type = Transient
  scheme = bdf2
  start_time = -200.000002
  end_time = 70
  nl_rel_tol = 1e-6
  nl_abs_tol = 1e-8
  solve_type = 'NEWTON'
  petsc_options = '-snes_converged_reason -ksp_converged_reason -snes_linesearch_monitor'
  petsc_options_iname = '-pc_type -pc_factor_shift_type -pc_factor_mat_solver_type'
  petsc_options_value = 'lu       NONZERO               superlu_dist'
  line_search = 'none'
  nl_max_its = 20
  nl_forced_its = 2
  l_max_its = 50

  automatic_scaling = true
  compute_scaling_once = false
  resid_vs_jac_scaling_param = 0.1

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
[]

[Outputs]
  perf_graph = true
  print_linear_residuals = true
  [./out]
    type = Exodus
    execute_on = timestep_end
  [../]
[]

[Debug]
  show_var_residual_norms = true
[]
