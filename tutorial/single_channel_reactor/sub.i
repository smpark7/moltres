flow_velocity=112.75
pre_scale=1e-12    # precursor scaling factor
ini_temp = 953

[GlobalParams]
  num_groups = 0
  num_precursor_groups = 8
  temperature = temp
  use_exp_form = false
  group_fluxes = ''
  sss2_input = true
  pre_concs = 'pre1 pre2 pre3 pre4 pre5 pre6 pre7 pre8'
[]

[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 376
  xmax = 188
  elem_type = EDGE2
[../]

[Variables]
  [./temp]
    order = CONSTANT
    family = MONOMIAL
    scaling = 1e-3
    initial_condition = ${ini_temp}
  [../]
[]

[Precursors]
  [./core]
    var_name_base = pre
    outlet_boundaries = 'right'
    u_def = 0
    v_def = ${flow_velocity}
    w_def = 0
    nt_exp_form = false
    family = MONOMIAL
    order = CONSTANT
    loop_precs = true
    multi_app = loopApp
    is_loopapp = true
    inlet_boundaries = 'left'
    scaling = ${pre_scale}
  [../]
[]

[Kernels]
  [./temp_time_derivative]
    type = MatINSTemperatureTimeDerivative
    variable = temp
  [../]
[]

[DGKernels]
  [./temp_adv]
    type = DGTemperatureAdvection
    variable = temp
    velocity = '${flow_velocity} 0 0'
  [../]
[]

[DiracKernels]
  [./heat_exchanger]
    type = DiracHX
    variable = temp
    power = 7.4135e4   # Power to reduce temp by 100 K, Q = m c_p T
    point = '94 0 0'
  [../]
[]

[BCs]
  [./fuel_bottom]
    boundary = 'left'
    type = PostprocessorTemperatureInflowBC
    postprocessor = coreEndTemp
    variable = temp
    uu = ${flow_velocity}
  [../]
  [./temp_advection_outflow]
    boundary = 'right'
    type = TemperatureOutflowBC
    variable = temp
    velocity = '${flow_velocity} 0 0'
  [../]
[]

[Materials]
  [./fuel]
    type = MoltresJsonMaterial
    base_file = 'msfrXS.json'
    material_key = 'fuel'
    interp_type = 'spline'
    prop_names = 'cp rho k'
    prop_values = '1594 4.125e-3 1.010e-2'
  [../]
[]

[Executioner]
  type = Transient
  end_time = 100

  nl_rel_tol = 1e-6
  nl_abs_tol = 1e-6

  solve_type = 'NEWTON'
  petsc_options = '-snes_converged_reason -ksp_converged_reason -snes_linesearch_monitor'
  petsc_options_iname = '-pc_type -pc_factor_shift_type'
  petsc_options_value = 'lu       NONZERO'
  line_search = 'none'
   # petsc_options_iname = '-snes_type'
  # petsc_options_value = 'test'

  nl_max_its = 30
  l_max_its = 100

  dtmin = 1e-5
  # dtmax = 1
  # dt = 1e-3
  [./TimeStepper]
    type = IterationAdaptiveDT
    dt = 1e-5
    cutback_factor = .4
    growth_factor = 1.2
    optimal_iterations = 25
  [../]
[]

[Preconditioning]
  [./SMP]
    type = SMP
    full = true
  [../]
[]

[Postprocessors]
  [./temp_loop]
    type = ElementAverageValue
    variable = temp
    outputs = 'exodus console'
  [../]
  [./loopEndTemp]
    type = SideAverageValue
    variable = temp
    boundary = 'right'
  [../]
  [./coreEndTemp]
    type = Receiver
  [../]
  [./loopMinTemp]
    type = ElementExtremeValue
    value_type = min
    variable = temp
    outputs = 'exodus console'
  [../]
[]

[Outputs]
  perf_graph = true
  print_linear_residuals = true
  [./exodus]
    type = Exodus
    execute_on = 'timestep_end'
  [../]
[]

[Debug]
  show_var_residual_norms = true
[]
