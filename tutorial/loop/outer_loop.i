flow_velocity = 21.333 # cm/s

[GlobalParams]
  num_groups = 0
  num_precursor_groups = 6
  temperature = temp
  group_fluxes = ''
  sss2_input = true
[]

[Mesh]
  [outer_loop]
    type = GeneratedMeshGenerator
    dim = 1
    nx = 270
    xmax = 270
    elem_type = EDGE2
  []
  [heat_exchanger]
    type = SubdomainBoundingBoxGenerator
    input = outer_loop
    bottom_left = '115 0 0'
    top_right = '155 0 0'
    block_id = 1
  []
[]

[Variables]
  [temp]
    initial_condition = 940
  []
[]

[Precursors]
  [outer_loop]
    var_name_base = pre
    outlet_boundaries = 'right'
    u_def = ${flow_velocity}
    v_def = 0
    w_def = 0
    nt_exp_form = false
    family = MONOMIAL
    order = CONSTANT
    loop_precursors = true
    multi_app = loop_app
    is_loopapp = true
    inlet_boundaries = 'left'
  []
[]

[Kernels]
  [temp_time_derivative]
    type = INSTemperatureTimeDerivative
    variable = temp
  []
  [temp_advection]
    type = ConservativeTemperatureAdvection
    variable = temp
    velocity = '${flow_velocity} 0 0'
  []
  [temp_diffusion]
    type = MatDiffusion
    variable = temp
    diffusivity = 'k'
  []
  [heat_exchanger]
    type = ConvectiveHeatExchanger
    variable = temp
    block = 1
    htc = 0
    tref = 800
  []
[]

[BCs]
  [loop_inlet]
    type = PostprocessorDirichletBC
    variable = temp
    boundary = 'left'
    postprocessor = loop_inlet_temp
  []
  [loop_outlet]
    type = TemperatureOutflowBC
    variable = temp
    boundary = 'right'
    velocity = '${flow_velocity} 0 0'
  []
[]

[Controls]
  [heat_exchanger_htc]
    type = RealFunctionControl
    parameter = '*/*/htc'
    function = htc_func
  []
[]

[Functions]
  [htc_func]
    type = ParsedFunction
    expression = '.52 * tanh(t)'
  []
[]

[Materials]
  [fuel]
    type = MoltresJsonMaterial
    base_file = '../eigenvalue/xsdata.json'
    material_key = 'fuel'
    interp_type = LINEAR
    prop_names = 'rho k cp'
    prop_values = '2.146e-3 .0553 1967'
  []
[]

[Executioner]
  type = Transient
  end_time = 400
  nl_rel_tol = 1e-6
  nl_abs_tol = 1e-5
  solve_type = 'NEWTON'
  petsc_options = '-snes_converged_reason -ksp_converged_reason -snes_linesearch_monitor'
  petsc_options_iname = '-pc_type -pc_factor_shift_type'
  petsc_options_value = 'lu       NONZERO'
  line_search = 'none'
  nl_max_its = 20
  l_max_its = 50
  dtmin = 1e-2
  dtmax = 4
  [TimeStepper]
    type = IterationAdaptiveDT
    dt = 1e-2
    cutback_factor = 0.4
    growth_factor = 1.2
    optimal_iterations = 20
  []
[]

[Preconditioning]
  [SMP]
    type = SMP
    full = true
  []
[]

[Postprocessors]
  [loop_inlet_temp]
    type = Receiver
    default = 940
  []
  [loop_outlet_temp]
    type = SideAverageValue
    variable = temp
    boundary = 'right'
  []
[]

[Outputs]
  perf_graph = true
  [exodus]
    type = Exodus
  []
[]

[Debug]
[]
