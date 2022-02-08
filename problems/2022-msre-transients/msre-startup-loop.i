[GlobalParams]
  num_groups = 0
  num_precursor_groups = 6
  temperature = 922
  group_fluxes = ''
  sss2_input = false
[]

[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 100
  xmax = 350
  elem_type = EDGE2
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
  [../]
[]

[Functions]
  [./vel_func]
    type = ParsedFunction
    value = 'if(t<10.0, 21.45 * 0.01 * (100 * ( 0.38972176921688984 / (1 + exp(- 4.142965323851681 * (t - 2.1997827088970765 ))) + 0.13469852389267972 / (1 + exp(- 15.999999999891843 * (t - 1.8229607556265044 ))) + 0.024364484449346135 / (1 + exp(- 3.3040453278344293 * (t - 8.261191902527791 ))) + 0.13064766354202037 / (1 + exp(- 15.999999999991578 * (t - 1.427099833340688 )))) + 15.999999999999998 / (1 + exp(- 3.160549381680319 * (t - 3.0457517232294262 ))) + 15.999999999999998 / (1 + exp(- 1.7598149179499631 * (t - 4.365843500245082 )))), 21.45)'
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
  end_time = 50
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
