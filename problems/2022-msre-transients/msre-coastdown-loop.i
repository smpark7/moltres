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
    file = 'msre-steady-state-flow_out_loopApp0_out.e'
    use_for_exodus_restart = true
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
    order = CONSTANT
    loop_precursors = true
    multi_app = loopApp
    is_loopapp = true
    inlet_boundaries = 'left'
    init_from_file = true
  [../]
[]

[Functions]
  [./vel_func]
    type = ParsedFunction
    value = 'if(t<20.0, 21.45 * 0.01 * (100 * ( 0.08567004393586444 / (1 + exp( 0.18270364351392387 * (t - 11.348385006979589 ))) + 0.11212706244321342 / (1 + exp( 0.6423926745267268 * (t - 8.720664295533949 ))) + 0.48132650370444385 / (1 + exp( 2.1187238480013177 * (t - 2.746163100638864 ))) + 0.21846374585678544 / (1 + exp( 3.1513341812734037 * (t - 5.142977826914452 )))) + 11.385268805267097 / (1 + exp( 7.028373826005297 * (t - 1.1677113693268026 ))) - 1.322249566482958 / (1 + exp(- 15.020553937112997 * (t - 4.482602484800283 )))), 1e-14)'
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
  end_time = 70
  nl_rel_tol = 1e-6
  nl_abs_tol = 1e-8
  solve_type = 'NEWTON'
  petsc_options = '-snes_converged_reason -ksp_converged_reason -snes_linesearch_monitor'
  petsc_options_iname = '-pc_type -pc_factor_shift_type -pc_factor_mat_solver_type'
  petsc_options_value = 'lu       NONZERO               superlu_dist'
  line_search = 'none'
  nl_max_its = 20
  l_max_its = 50

  automatic_scaling = true
  compute_scaling_once = false
  resid_vs_jac_scaling_param = 0.1

  dt = 1
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
