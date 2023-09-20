flow_velocity = 21.333

[GlobalParams]
  num_groups = 2
  num_precursor_groups = 6
  use_exp_form = false
  group_fluxes = 'group1 group2'
  pre_concs = 'pre1 pre2 pre3 pre4 pre5 pre6'
  temperature = temp
  sss2_input = true
  account_delayed = true
  eigenvalue_scaling = 1.0205
[]

[Mesh]
  coord_type = RZ
  [mesh]
    type = FileMeshGenerator
    file = '../eigenvalue/mesh.e'
  []
[]

[Variables]
  [temp]
    order = FIRST
    family = LAGRANGE
    initial_condition = 940
  []
[]

[Nt]
  var_name_base = group
  vacuum_boundaries = 'fuel_bottom mod_bottom right fuel_top mod_top'
  pre_blocks = '0'
  create_temperature_var = false
  nt_ic_function = ic_func
  eigen = false
[]

[Precursors]
  [pres]
    var_name_base = pre
    family = MONOMIAL
    order = CONSTANT
    block = 0
    outlet_boundaries = 'fuel_top'
    inlet_boundaries = 'fuel_bottom'
    constant_velocity_values = true
    u_def = 0
    v_def = ${flow_velocity}
    w_def = 0
    nt_exp_form = false
    loop_precursors = true
    multi_app = loop_app
    is_loopapp = false
    transient = true
  []
[]

[Kernels]
  [temp_time_derivative]
    type = INSTemperatureTimeDerivative
    variable = temp
  []
  [temp_advection_fuel]
    type = ConservativeTemperatureAdvection
    variable = temp
    velocity = '0 ${flow_velocity} 0'
    block = '0'
  []
  [temp_diffusion]
    type = MatDiffusion
    variable = temp
    diffusivity = 'k'
  []
  [temp_source_fuel]
    type = TransientFissionHeatSource
    variable = temp
    block = '0'
  []
[]

[BCs]
  [temp_inlet_bc]
    type = PostprocessorDirichletBC
    variable = temp
    boundary = 'fuel_bottom right'
    postprocessor = inlet_temp
  []
  [temp_outlet_bc]
    type = TemperatureOutflowBC
    variable = temp
    boundary = 'fuel_top'
    velocity = '0 ${flow_velocity} 0'
  []
[]

[Functions]
  [temp_bc_func]
    type = ParsedFunction
    expression = '940 - (940-908) * tanh(t/1)'
  []
  [ic_func]
    type = ParsedFunction
    expression = '1e5 * (-x^2+70^2) * (-y * (y-160))'
  []
[]

[Materials]
  [fuel]
    type = MoltresJsonMaterial
    block = '0'
    base_file = '../eigenvalue/xsdata.json'
    material_key = 'fuel'
    interp_type = LINEAR
    prop_names = 'rho k cp'
    prop_values = '2.146e-3 .0553 1967'
  []
  [graphite]
    type = MoltresJsonMaterial
    block = '1'
    base_file = '../eigenvalue/xsdata.json'
    material_key = 'graphite'
    interp_type = LINEAR
    prop_names = 'rho k cp'
    prop_values = '1.86e-3 .312 1760'
  []
[]

[MultiApps]
  [loop_app]
    type = TransientMultiApp
    app_type = MoltresApp
    execute_on = timestep_begin
    input_files = 'outer_loop.i'
  []
[]

[Transfers]
  [from_outer_loop]
    type = MultiAppPostprocessorTransfer
    from_multi_app = loop_app
    from_postprocessor = loop_outlet_temp
    to_postprocessor = inlet_temp
    reduction_type = maximum
  []
  [to_outer_loop]
    type = MultiAppPostprocessorTransfer
    to_multi_app = loop_app
    from_postprocessor = outlet_temp
    to_postprocessor = loop_inlet_temp
  []
[]

[Executioner]
  type = Transient
  end_time = 400

  nl_rel_tol = 1e-6
  nl_abs_tol = 1e-1

  automatic_scaling = true
  compute_scaling_once = false
  resid_vs_jac_scaling_param = 0.1
  scaling_group_variables = 'group1 group2; pre1 pre2 pre3 pre4 pre5 pre6; temp'

  steady_state_detection = true
  steady_state_tolerance = 1e-6
  steady_state_start_time = 200

  solve_type = 'NEWTON'
  petsc_options = '-snes_converged_reason -ksp_converged_reason -snes_linesearch_monitor'
  petsc_options_iname = '-pc_type -pc_factor_shift_type -pc_factor_mat_solver_package'
  petsc_options_value = 'lu       NONZERO               superlu_dist'

  line_search = none

  dtmin = 1e-2
  dtmax = 2
#  [TimeStepper]
#    type = FunctionDT
#    function = dt_func
#  []
  [TimeStepper]
    type = IterationAdaptiveDT
    dt = 1e-2
    cutback_factor = 0.4
    growth_factor = 1.2
    optimal_iterations = 5
    iteration_window = 1
  []
[]

[Preconditioning]
  [SMP]
    type = SMP
    full = true
  []
[]

[Postprocessors]
  [tot_fissions]
    type = ElmIntegTotFissPostprocessor
    execute_on = linear
  []
  [powernorm]
    type = ElmIntegTotFissHeatPostprocessor
    execute_on = linear
  []
  [average_temp]
    type = ElementAverageValue
    variable = temp
    execute_on = linear
  []
  [inlet_temp]
    type = Receiver
  []
  [outlet_temp]
    type = SideAverageValue
    variable = temp
    boundary = fuel_top
    execute_on = 'initial timestep_end'
  []
[]

[VectorPostprocessors]
  [centerline_flux]
    type = LineValueSampler
    variable = 'group1 group2'
    start_point = '0 0 0'
    end_point = '0 160 0'
    num_points = 161
    sort_by = y
    execute_on = FINAL
  []
  [midplane_flux]
    type = LineValueSampler
    variable = 'group1 group2'
    start_point = '0 80 0'
    end_point = '69.4 80 0'
    num_points = 100
    sort_by = x
    execute_on = FINAL
  []
[]

[Outputs]
  perf_graph = true
  [exodus]
    type = Exodus
  []
  [csv]
    type = CSV
    execute_on = FINAL
  []
[]

[Debug]
  show_var_residual_norms = true
[]
