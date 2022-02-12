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
  nx = 70
  xmax = 350
  elem_type = EDGE2
[../]

[Precursors]
  [./core]
    var_name_base = pre
    outlet_boundaries = 'right'
    u_def = 21.45
    v_def = 0
    w_def = 0
    nt_exp_form = false
    family = MONOMIAL
    order = CONSTANT
    loop_precursors = true
    multi_app = loopApp
    is_loopapp = true
    inlet_boundaries = 'left'
  [../]
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
  end_time = 2000
  nl_rel_tol = 1e-6
  nl_abs_tol = 1e-8
  solve_type = 'NEWTON'
  petsc_options = '-snes_converged_reason -ksp_converged_reason -snes_linesearch_monitor'
  petsc_options_iname = '-pc_type -pc_factor_shift_type'
  petsc_options_value = 'lu       NONZERO'
  line_search = 'none'
  nl_max_its = 20
  nl_forced_its = 2
  l_max_its = 50

  automatic_scaling = true
  compute_scaling_once = false
  resid_vs_jac_scaling_param = 0.1

  dtmin = 1
  dtmax = 20
  [./TimeStepper]
    type = IterationAdaptiveDT
    dt = 1
    cutback_factor = .5
    growth_factor = 1.5
    optimal_iterations = 1000
    iteration_window = 4
    linear_iteration_ratio = 1000
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
