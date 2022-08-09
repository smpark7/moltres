[GlobalParams]
  num_groups = 2
  num_precursor_groups = 1
  group_fluxes = '0 0'
  pre_concs = 'pre1'
  temperature = 922
  transient = true
  integrate_p_by_parts = true
  account_delayed = true
[]

[Mesh]
  type = GeneratedMesh
  dim = 3
  nx = 136
  ny = 1
  nz = 1
  xmax = 170
#  elem_type = EDGE2
[]

[Problem]
  type = FEProblem
[]

[Precursors]
  [./pres]
    var_name_base = pre
    outlet_boundaries = 'right'
    constant_velocity_values = true
    u_def = 20.8
    v_def = 0
    w_def = 0
    nt_exp_form = false
    family = L2_LAGRANGE
    order = SECOND
    loop_precursors = true
    multi_app = loopApp
    is_loopapp = false
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

[Functions]
  [./ic_func]
    type = ParsedFunction
#    value = 'if(x<70, if(x>20, 2, 0), 0)'
    value = 'exp(-((x-85)^2)/2/(20^2))/20/sqrt(2*pi)'
  []
[]

[ICs]
  [./pre1]
    type = FunctionIC
    variable = 'pre1'
    function = ic_func
  []
[]

[Materials]
  [./fuel_nts]
    type = MoltresJsonMaterial
    base_file = 'single-channel.json'
    material_key = 'fuel'
    interp_type = 'NONE'
    prop_names = ''
    prop_values = ''
  [../]
[]

[Executioner]
  type = Transient
  end_time = .05

  solve_type = 'NEWTON'
  petsc_options = '-snes_converged_reason -ksp_converged_reason -snes_linesearch_monitor'
#  petsc_options_iname = '-pc_type -sub_pc_type -ksp_gmres_restart -pc_asm_overlap -sub_pc_factor_shift_type'
#  petsc_options_value = 'asm      lu           200                1               NONZERO'
  petsc_options_iname = '-pc_type -pc_factor_shift_type -pc_factor_mat_solver_type'
  petsc_options_value = 'lu       NONZERO               superlu_dist'

  nl_abs_tol = 1e-10
  nl_forced_its = 1
  l_tol = 1e-5
  l_max_its = 200
  line_search = none
  automatic_scaling = true
  compute_scaling_once = false
  resid_vs_jac_scaling_param = 0.1

  auto_advance = true
  fixed_point_abs_tol = 1e-7
  fixed_point_max_its = 5

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
  [./average]
    type = ElementAverageValue
    variable = pre1
  []
  [./max]
    type = ElementExtremeValue
    variable = pre1
  []
[]

[MultiApps]
  [./loopApp]
    type = TransientMultiApp
    app_type = MoltresApp
    execute_on = timestep_begin
    positions = '1000 0 0'
    input_files = 'sub.i'
  []
[]

[Outputs]
  perf_graph = true
  print_linear_residuals = false
  [./exodus]
    type = Exodus
  []
  [./csv]
    type = CSV
  []
[]

[Debug]
  show_var_residual_norms = false
[]
