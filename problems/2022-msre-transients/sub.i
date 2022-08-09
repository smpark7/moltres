[GlobalParams]
  num_groups = 0
  num_precursor_groups = 1
  temperature = 922
  group_fluxes = ''
  sss2_input = false
[]

[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 280
  xmax = 350
  elem_type = EDGE2
[../]

[Precursors]
  [./core]
    var_name_base = pre
    outlet_boundaries = 'right'
    constant_velocity_values = true
    u_def = 20.8
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

[Kernels]
  [./convection]
    type = ConservativeAdvection
    variable = pre1
    velocity = '20.8 0 0'
  []
[]

[Materials]
  [./fuel_nts]
    type = MoltresJsonMaterial
    block = '0'
    base_file = 'single-channel.json'
    material_key = 'fuel'
    interp_type = 'NONE'
    prop_names = ''
    prop_values = ''
  [../]
[]

[Executioner]
  type = Transient
  end_time = 25
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

  dt = .05
  [./TimeIntegrator]
#    type = LStableDirk3
    type = BDF2
  []
[]

[Preconditioning]
  [./SMP]
    type = SMP
    full = true
  [../]
[]

[Postprocessors]
  [./sub_average]
    type = ElementAverageValue
    variable = pre1
  []
[]

[Outputs]
  perf_graph = true
  print_linear_residuals = false
  [./out]
    type = Exodus
    execute_on = timestep_end
  [../]
[]

[Debug]
  show_var_residual_norms = false
[]
