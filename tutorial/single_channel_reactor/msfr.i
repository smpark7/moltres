flow_velocity=112.75 # cm/s
nt_scale=1e-15     # neutron flux scaling factor
pre_scale=1e-12    # precursor scaling factor
temp_scale = 1e-3
ini_temp=953     # initial temp
diri_temp=923    # dirichlet BC temp
ini_neut=1e13

[GlobalParams]
  num_groups = 6
  num_precursor_groups = 8
  temperature = temp
  group_fluxes = 'group1 group2 group3 group4 group5 group6'
  pre_concs = 'pre1 pre2 pre3 pre4 pre5 pre6 pre7 pre8'
  use_exp_form = false
  sss2_input = true
  account_delayed = true
[]

[Mesh]
  file = 'msfr-tutorial.e'
[../]

[Problem]
  type = FEProblem
  coord_type = RZ
  rz_coord_axis = Y
[]

[Nt]
  var_name_base = group
  create_temperature_var = true
  temp_scaling = ${temp_scale}
  nt_ic_function = ntICFunc
  vacuum_boundaries = 'fuel_top blanket_top fuel_bottom blanket_bottom outer'
  dg_for_temperature = false
  eigen = false
  pre_blocks = fuel
  scaling = ${nt_scale}
[]

[Precursors]
  [./pres]
    var_name_base = pre
    block = 'fuel'
    outlet_boundaries = 'fuel_top'
    constant_velocity_values = true
    u_def = 0
    v_def = ${flow_velocity}
    w_def = 0
    nt_exp_form = false
    family = MONOMIAL
    order = CONSTANT
    transient = true
    loop_precs = true
    multi_app = loopApp
    is_loopapp = false
    inlet_boundaries = 'fuel_bottom'
    scaling = ${pre_scale}
  [../]
[]

[Kernels]
  # Temperature
  [./temp_time_derivative]
    type = MatINSTemperatureTimeDerivative
    variable = temp
  [../]
  [./temp_cond]
    type = MatDiffusion
    variable = temp
    D_name = 'k'
  [../]
  [./temp_source]
    type = TransientFissionHeatSource
    nt_scale=1
    variable = temp
  [../]
  [./temp_advection_fuel]
    type = ConservativeTemperatureAdvection
    velocity = '0 ${flow_velocity} 0'
    variable = temp
    block = 'fuel'
  [../]
[]

[Functions]
  [ntICFunc]
    type = ConstantFunction
    value = ${ini_neut}
  [../]
  [temp_bc_func]
    type = ParsedFunction
    value = '${ini_temp} - (${ini_temp} - ${diri_temp}) * tanh(t/50)'
  [../]
[]

[Materials]
  [./fuel]
    type = MoltresJsonMaterial
    base_file = 'msfrXS.json'
    block = 'fuel'
    material_key = 'fuel'
    interp_type = 'spline'
    prop_names = 'cp rho k'
    prop_values = '1594 4.125e-3 1.010e-2'
  [../]
  [./blanket]
    type = MoltresJsonMaterial
    base_file = 'msfrXS.json'
    block = 'blanket'
    material_key = 'blanket'
    interp_type = 'spline'
    prop_names = 'cp rho k'
    prop_values = '1594 4.125e-3 10'
  [../]
[]

[BCs]
  [./temp_diri]
    boundary = 'blanket_bottom outer'
    type = FunctionDirichletBC
    variable = temp
    function = temp_bc_func
  [../]
  [./temp_fuel_bottom]
    boundary = 'fuel_bottom'
    type = PostprocessorDirichletBC
    postprocessor = inlet_mean_temp
    variable = temp
  [../]
  [./temp_advection_outlet]
    boundary = 'fuel_top'
    type = TemperatureOutflowBC
    variable = temp
    velocity = '0 ${flow_velocity} 0'
  [../]
[]

[Executioner]
  type = Transient
  end_time = 100

  nl_rel_tol = 1e-6
  nl_abs_tol = 1e-5

  solve_type = 'NEWTON'
  petsc_options = '-snes_converged_reason -ksp_converged_reason -snes_linesearch_monitor'
  petsc_options_iname = '-pc_type -pc_factor_shift_type'
  petsc_options_value = 'lu       NONZERO'
  line_search = 'none'

  nl_max_its = 30
  l_max_its = 100

  picard_max_its = 5
  picard_rel_tol = 1e-6
  picard_abs_tol = 1e-5

  dtmin = 1e-5
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
  [./group1_current]
    type = IntegralNewVariablePostprocessor
    variable = group1
    outputs = 'console exodus csv'
  [../]
  [./group2_current]
    type = IntegralNewVariablePostprocessor
    variable = group2
    outputs = 'console exodus csv'
  [../]
  [./group3_current]
    type = IntegralNewVariablePostprocessor
    variable = group3
    outputs = 'console exodus csv'
  [../]
  [./group4_current]
    type = IntegralNewVariablePostprocessor
    variable = group4
    outputs = 'console exodus csv'
  [../]
  [./group5_current]
    type = IntegralNewVariablePostprocessor
    variable = group5
    outputs = 'console exodus csv'
  [../]
  [./group6_current]
    type = IntegralNewVariablePostprocessor
    variable = group6
    outputs = 'console exodus csv'
  [../]
  [./temp_fuel]
    type = ElementAverageValue
    variable = temp
    block = 'fuel'
    outputs = 'exodus console csv'
  [../]
  [./max_temp_fuel]
    type = ElementExtremeValue
    variable = temp
    block = 'fuel'
    value_type = 'max'
    outputs = 'exodus console csv'
  [../]
  [./temp_blanket]
    type = ElementAverageValue
    variable = temp
    block = 'blanket'
    outputs = 'exodus console csv'
  [../]
  [./heat_fuel]
    type = ElmIntegTotFissHeatPostprocessor
    block = 'fuel'
    outputs = 'exodus'
  [../]
  [./heat_blanket]
    type = ElmIntegTotFissHeatPostprocessor
    block = 'blanket'
    outputs = 'exodus'
  [../]
  [./heat]
    type = ElmIntegTotFissHeatPostprocessor
    outputs = 'csv'
  [../]
  [./coreEndTemp]
    type = SideAverageValue
    variable = temp
    boundary = 'fuel_top'
    outputs = 'exodus console'
  [../]
  [./inlet_mean_temp]
    type = Receiver
    default = 930
    initialize_old = true
    execute_on = 'timestep_begin'
    outputs = 'csv console'
  [../]
[]

[Outputs]
  perf_graph = true
  csv = true
  [./exodus]
    type = Exodus
    execute_on = 'timestep_end'
  [../]
[]

[Debug]
  show_var_residual_norms = true
[]

[MultiApps]
  [./loopApp]
    type = TransientMultiApp
    app_type = MoltresApp
    execute_on = timestep_begin
    positions = '200.0 200.0 0.0'
    input_files = 'sub.i'
  [../]
[]

[Transfers]
  [./from_loop]
    type = MultiAppPostprocessorTransfer
    multi_app = loopApp
    from_postprocessor = loopEndTemp
    to_postprocessor = inlet_mean_temp
    direction = from_multiapp
    reduction_type = maximum
  [../]
  [./to_loop]
    type = MultiAppPostprocessorTransfer
    multi_app = loopApp
    from_postprocessor = coreEndTemp
    to_postprocessor = coreEndTemp
    direction = to_multiapp
  [../]
[]

[ICs]
  [./temp_ic]
    type = ConstantIC
    variable = temp
    value = ${ini_temp}
  [../]
[]
