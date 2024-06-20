[GlobalParams]
  num_groups = 0
  num_precursor_groups = 6
  group_fluxes = ''
  temperature = temp
  sss2_input = true
[]

[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 1
    nx = 250
    xmin = 0
    xmax = 250
  []
  [heat_exchanger]
    type = SubdomainBoundingBoxGenerator
    input = gmg
    block_id = 1
    bottom_left = '150 0 0'
    top_right = '200 0 0'
  []
[]

[Variables]
  [temp]
    order = FIRST
    family = LAGRANGE
    initial_condition = 970
  []
[]

[Precursors]
  [pres]
    var_name_base = pre
    family = MONOMIAL
    order = FIRST
    inlet_boundaries = 'left'
    outlet_boundaries = 'right'
    constant_velocity_values = true
    u_def = 18.085
    v_def = 0
    w_def = 0
    nt_exp_form = false
    loop_precursors = true
    multi_app = loop_app
    is_loopapp = true
    transient = true
  []
[]

[Kernels]
  [temp_time_derivative]
    type = MatINSTemperatureTimeDerivative
    variable = temp
  []
  [temp_advection_fuel]
    type = ConservativeTemperatureAdvection
    variable = temp
    velocity = '18.085 0 0'
  []
  [temp_diffusion]
    type = MatDiffusion
    variable = temp
    diffusivity = 'k'
  []
  [temp_heat_sink]
    type = ConvectiveHeatExchanger
    variable = temp
    block = 1
    htc = .46
    tref = 900
  []

  [pre1_advection]
    type = ConservativeAdvection
    variable = pre1
    velocity = '18.085 0 0'
  []
  [pre2_advection]
    type = ConservativeAdvection
    variable = pre2
    velocity = '18.085 0 0'
  []
  [pre3_advection]
    type = ConservativeAdvection
    variable = pre3
    velocity = '18.085 0 0'
  []
  [pre4_advection]
    type = ConservativeAdvection
    variable = pre4
    velocity = '18.085 0 0'
  []
  [pre5_advection]
    type = ConservativeAdvection
    variable = pre5
    velocity = '18.085 0 0'
  []
  [pre6_advection]
    type = ConservativeAdvection
    variable = pre6
    velocity = '18.085 0 0'
  []
[]

[BCs]
  [temp_inlet_bc]
    type = PostprocessorTemperatureInflowBC
    variable = temp
    boundary = 'left'
    postprocessor = loop_inlet_temp
    uu = 18.085
    vv = 0 # optional, defaults to 0
    ww = 0 # optional, defaults to 0
  []
  [temp_outlet_bc]
    type = TemperatureOutflowBC
    variable = temp
    boundary = 'right'
    velocity = '18.085 0 0'
  []
[]

[Functions]
  [dt_func]
    type = ParsedFunction
    expression = 'if(t<30, .4, 5)'
  []
  [ic_func]
    type = ParsedFunction
    expression = '1e5 * (-x^2+70^2) * (-y * (y-150))'
  []
[]

[Materials]
  [fuel]
    type = MoltresJsonMaterial
    block = '0 1'
    base_file = '../eigenvalue/xsdata.json'
    material_key = 'fuel'
    interp_type = LINEAR
    prop_names = 'rho k cp'
    prop_values = '2.146e-3 .0553 1967'
  []
[]

[Executioner]
  type = Transient
  end_time = 150

  nl_rel_tol = 1e-6
  nl_abs_tol = 1e-2

  automatic_scaling = true
  compute_scaling_once = false
  off_diagonals_in_auto_scaling = true
  resid_vs_jac_scaling_param = 0.1
  scaling_group_variables = 'pre1 pre2 pre3 pre4 pre5 pre6; temp'

  solve_type = 'NEWTON'
  petsc_options = '-snes_converged_reason -ksp_converged_reason -snes_linesearch_monitor'
  petsc_options_iname = '-pc_type -pc_factor_shift_type -pc_factor_mat_solver_package'
  petsc_options_value = 'lu       NONZERO               superlu_dist'

  line_search = none

  dtmin = 1e-3
  dtmax = 10
  [TimeStepper]
    type = FunctionDT
    function = dt_func
  []
  #  [TimeStepper]
  #    type = IterationAdaptiveDT
  #    dt = .2
  #    cutback_factor = 0.4
  #    growth_factor = 1.2
  #    optimal_iterations = 20
  #  []
[]

[Preconditioning]
  [SMP]
    type = SMP
    full = true
  []
[]

[Postprocessors]
  [average_temp]
    type = ElementAverageValue
    variable = temp
    execute_on = linear
  []
  [loop_inlet_temp]
    type = Receiver
    default = 1000
  []
  [loop_outlet_temp]
    type = SideAverageValue
    variable = temp
    boundary = right
  []
[]

[Outputs]
  perf_graph = true
  #  print_linear_residuals = true
  [exodus]
    type = Exodus
  []
  [csv]
    type = CSV
    execute_on = FINAL
  []
[]

[Debug]
#  show_var_residual_norms = true
[]
