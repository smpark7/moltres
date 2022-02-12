[GlobalParams]
  num_groups = 2
  num_precursor_groups = 6
  group_fluxes = 'group1 group2'
  pre_concs = 'pre1 pre2 pre3 pre4 pre5 pre6'
  temperature = 922
  sss2_input = true
  transient = true
  integrate_p_by_parts = true
  account_delayed = true
[]

[Mesh]
  [./mesh]
    type = FileMeshGenerator
    file = 'msre-steady-state-flow_exodus.e'
    use_for_exodus_restart = true
  []
[]

[Problem]
  type = FEProblem
  coord_type = RZ
  kernel_coverage_check = false
[]

[Precursors]
  [./pres]
    var_name_base = pre
    outlet_boundaries = 'fuel_top'
    outlet_vel = vel_y
    constant_velocity_values = false
    uvel = vel_x
    vvel = vel_y
    nt_exp_form = false
    family = MONOMIAL
    order = CONSTANT
    loop_precursors = true
    multi_app = loopApp
    is_loopapp = false
    inlet_boundaries = 'fuel_bottom'
    block = 0
    init_from_file = true
  [../]
[]

[Variables]
  [./vel]
    family = LAGRANGE_VEC
    order = FIRST
    block = 0
  []
  [./p]
    family = LAGRANGE
    order = FIRST
    block = 0
  []
[]

[AuxVariables]
  [./vel_x]
    family = LAGRANGE
    order = FIRST
    block = 0
    initial_from_file_var = vel_x
    initial_from_file_timestep = LATEST
  []
  [./vel_y]
    family = LAGRANGE
    order = FIRST
    block = 0
    initial_from_file_var = vel_y
    initial_from_file_timestep = LATEST
  []
  [./group1]
    family = LAGRANGE
    order = FIRST
    initial_from_file_var = group1
    initial_from_file_timestep = LATEST
  []
  [./group2]
    family = LAGRANGE
    order = FIRST
    initial_from_file_var = group2
    initial_from_file_timestep = LATEST
  []
  [./group1_source]
    family = LAGRANGE
    order = FIRST
    initial_from_file_var = group1_source
    initial_from_file_timestep = LATEST
  []
  [./group2_source]
    family = LAGRANGE
    order = FIRST
    initial_from_file_var = group2_source
    initial_from_file_timestep = LATEST
  []
[]

[Kernels]
  [./mass]
    type = INSADMass
    variable = p
    block = 0
  []
  [./mass_pspg]
    type = INSADMassPSPG
    variable = p
    block = 0
  []
  [./momentum_advection]
    type = INSADMomentumAdvection
    variable = vel
    block = 0
  []
  [./momentum_viscous]
    type = INSADMomentumViscous
    variable = vel
    block = 0
  []
  [./momentum_pressure]
    type = INSADMomentumPressure
    variable = vel
    block = 0
    p = p
  []
[]

[AuxKernels]
  [./vel_x]
    type = VectorVariableComponentAux
    variable = vel_x
    vector_variable = vel
    block = 0
    component = x
  []
  [./vel_y]
    type = VectorVariableComponentAux
    variable = vel_y
    vector_variable = vel
    block = 0
    component = y
  []
  [./group1_normalization]
    type = NormalizationAux
    variable = group1
    source_variable = group1_source
    normalization = bnorm
    execute_on = linear
  []
  [./group2_normalization]
    type = NormalizationAux
    variable = group2
    source_variable = group2_source
    normalization = bnorm
    execute_on = linear
  []
[]

[Functions]
  [./vel_func]
    type = ParsedFunction
    value = 'if(t<20.0, 21.45 * 0.01 * (100 * ( 0.08567004393586444 / (1 + exp( 0.18270364351392387 * (t - 11.348385006979589 ))) + 0.11212706244321342 / (1 + exp( 0.6423926745267268 * (t - 8.720664295533949 ))) + 0.48132650370444385 / (1 + exp( 2.1187238480013177 * (t - 2.746163100638864 ))) + 0.21846374585678544 / (1 + exp( 3.1513341812734037 * (t - 5.142977826914452 )))) + 11.385268805267097 / (1 + exp( 7.028373826005297 * (t - 1.1677113693268026 ))) - 1.322249566482958 / (1 + exp(- 15.020553937112997 * (t - 4.482602484800283 )))), 1e-14)'
  []
[]

[ICs]
  [./vel_ic]
    type = VectorConstantIC
    x_value = 0
    y_value = 21.45
    variable = vel
  []
[]

[BCs]
  [./inlet]
    type = VectorFunctionDirichletBC
    variable = vel
    boundary = 'fuel_bottom'
    function_x = '0'
    function_y = vel_func
  []
  [./outlet]
    type = VectorFunctionDirichletBC
    variable = vel
    boundary = 'fuel_top'
    function_x = '0'
    function_y = vel_func
  []
[]

[Materials]
  [./fuel]
    type = INSADTauMaterial
    block = 0
    pressure = p
    velocity = vel
    alpha = 1
  []
  [./ad_mat]
    type = ADGenericConstantMaterial
    block = '0 1'
    prop_names = 'mu rho'
    prop_values = '1 1'
  []
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

  solve_type = 'NEWTON'
  petsc_options = '-snes_converged_reason -ksp_converged_reason -snes_linesearch_monitor'
#  petsc_options_iname = '-pc_type -sub_pc_type -ksp_gmres_restart -pc_asm_overlap -sub_pc_factor_shift_type'
#  petsc_options_value = 'asm      lu           200                1               NONZERO'
  petsc_options_iname = '-pc_type -pc_factor_shift_type -pc_factor_mat_solver_type'
  petsc_options_value = 'lu       NONZERO               superlu_dist'

  nl_abs_tol = 1e-8
  nl_forced_its = 1
  l_tol = 1e-5
  line_search = none
  automatic_scaling = true
  compute_scaling_once = false
  resid_vs_jac_scaling_param = 0.1

  auto_advance = true
  fixed_point_abs_tol = 1e-7
  fixed_point_max_its = 5

  dt = .5
[]

[Preconditioning]
  [./SMP]
    type = SMP
    full = true
  [../]
[]

[Postprocessors]
  [./vel_y]
    type = ElementAverageValue
    block = 0
    variable = vel_y
    execute_on = TIMESTEP_END
  []
  [./bnorm]
    type = Receiver
    default = 1
  []
[]

[MultiApps]
  [./loopApp]
    type = TransientMultiApp
    app_type = MoltresApp
    execute_on = timestep_begin
    positions = '100 0 0'
    input_files = 'msre-coastdown-loop.i'
  []
  [./ntsApp]
    type = FullSolveMultiApp
    app_type = MoltresApp
    execute_on = timestep_end
    positions = '0 0 0'
    input_files = 'msre-transient-nts.i'
  []
[]

[Transfers]
  [./to_nts_pre]
    type = MultiAppCopyTransfer
    direction = to_multiapp
    multi_app = ntsApp
    source_variable = 'pre1 pre2 pre3 pre4 pre5 pre6'
    variable = 'pre1 pre2 pre3 pre4 pre5 pre6'
  []
  [./from_nts_flux]
    type = MultiAppCopyTransfer
    direction = from_multiapp
    multi_app = ntsApp
    source_variable = 'group1 group2'
    variable = 'group1_source group2_source'
  []
  [./from_nts_k_eff]
    type = MultiAppPostprocessorTransfer
    direction = from_multiapp
    multi_app = ntsApp
    from_postprocessor = bnorm
    to_postprocessor = bnorm
    reduction_type = average
  []
[]

[Outputs]
  perf_graph = true
  print_linear_residuals = true
  [./exodus]
    type = Exodus
  []
  [./csv]
    type = CSV
  []
[]

[Debug]
  show_var_residual_norms = true
[]
