[GlobalParams]
  num_groups = 2
  num_precursor_groups = 6
  use_exp_form = false
  group_fluxes = 'group1 group2'
  pre_concs = 'pre1 pre2 pre3 pre4 pre5 pre6'
  temperature = 922
  transient = true
  account_delayed = true
[]

[Mesh]
  [./mesh]
    type = FileMeshGenerator
    file = 'mesh.e'
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
    outlet_boundaries = ''
    constant_velocity_values = true
    u_def = 0
    v_def = 0
    w_def = 0
    nt_exp_form = false
    family = MONOMIAL
    order = SECOND
    loop_precursors = false
    transient = false
    block = '0'
  []
[]

[AuxVariables]
  [./group1]
    family = LAGRANGE
    order = FIRST
  []
  [./group2]
    family = LAGRANGE
    order = FIRST
  []
  [./group1_source]
    family = LAGRANGE
    order = FIRST
  []
  [./group2_source]
    family = LAGRANGE
    order = FIRST
  []
[]

[AuxKernels]
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

[Materials]
  [./fuel]
    type = MoltresJsonMaterial
    block = '0'
    base_file = 'xsdata.json'
    material_key = 'fuel'
    interp_type = 'NONE'
    prop_names = ''
    prop_values = ''
  [../]
  [./mod]
    type = MoltresJsonMaterial
    block = '1'
    base_file = 'xsdata.json'
    material_key = 'mod'
    interp_type = 'NONE'
    prop_names = ''
    prop_values = ''
  [../]
[]

[Executioner]
  type = Transient
  end_time = 2000

  solve_type = 'NEWTON'
  petsc_options = '-snes_converged_reason -ksp_converged_reason -snes_linesearch_monitor'
  petsc_options_iname = '-pc_type -sub_pc_type -ksp_gmres_restart -pc_asm_overlap -sub_pc_factor_shift_type'
  petsc_options_value = 'asm      lu           200                1               NONZERO'

  nl_abs_tol = 1e-8
  nl_forced_its = 1
  line_search = none
  automatic_scaling = true
  compute_scaling_once = false
  resid_vs_jac_scaling_param = 0.1

  auto_advance = true
  fixed_point_abs_tol = 1e-7
  fixed_point_max_its = 5

  dtmin = 1
  dtmax = 20
  steady_state_detection = true
  steady_state_tolerance = 1e-10
  steady_state_start_time = 100
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
  [./bnorm]
    type = Receiver
    default = 1
  [../]
  [./memory]
    type = MemoryUsage
  [../]
[]

[VectorPostprocessors]
  [./group1_centerline]
    type= LineValueSampler
    variable = group1_source
    start_point = '0 0 0'
    end_point = '0 170 0'
    num_points = 69
    sort_by = y
    execute_on = final
  []
  [./group2_centerline]
    type= LineValueSampler
    variable = group2_source
    start_point = '0 0 0'
    end_point = '0 170 0'
    num_points = 69
    sort_by = y
    execute_on = final
  []
  [./group1_midplane]
    type = LineValueSampler
    variable = group1_source
    start_point = '0 85 0'
    end_point = '69.375 85 0'
    num_points = 889
    sort_by = x
    execute_on = final
  []
  [./group2_midplane]
    type = LineValueSampler
    variable = group2_source
    start_point = '0 85 0'
    end_point = '69.375 85 0'
    num_points = 889
    sort_by = x
    execute_on = final
  []
  [./pre1_centerline]
    type = LineValueSampler
    variable = pre1
    start_point = '0 1.25 0'
    end_point = '0 168.75 0'
    num_points = 68
    sort_by = y
    execute_on = final
  []
  [./pre2_centerline]
    type = LineValueSampler
    variable = pre2
    start_point = '0 1.25 0'
    end_point = '0 168.75 0'
    num_points = 68
    sort_by = y
    execute_on = final
  []
  [./pre3_centerline]
    type = LineValueSampler
    variable = pre3
    start_point = '0 1.25 0'
    end_point = '0 168.75 0'
    num_points = 68
    sort_by = y
    execute_on = final
  []
  [./pre4_centerline]
    type = LineValueSampler
    variable = pre4
    start_point = '0 1.25 0'
    end_point = '0 168.75 0'
    num_points = 68
    sort_by = y
    execute_on = final
  []
  [./pre5_centerline]
    type = LineValueSampler
    variable = pre5
    start_point = '0 1.25 0'
    end_point = '0 168.75 0'
    num_points = 68
    sort_by = y
    execute_on = final
  []
  [./pre6_centerline]
    type = LineValueSampler
    variable = pre6
    start_point = '0 1.25 0'
    end_point = '0 168.75 0'
    num_points = 68
    sort_by = y
    execute_on = final
  []
  [./pre1_midplane]
    type = LineValueSampler
    variable = pre1
    start_point = '0.0390625 85 0'
    end_point = '69.3359375 85 0'
    num_points = 888
    sort_by = x
    execute_on = final
  []
  [./pre2_midplane]
    type = LineValueSampler
    variable = pre2
    start_point = '0.0390625 85 0'
    end_point = '69.3359375 85 0'
    num_points = 888
    sort_by = x
    execute_on = final
  []
  [./pre3_midplane]
    type = LineValueSampler
    variable = pre3
    start_point = '0.0390625 85 0'
    end_point = '69.3359375 85 0'
    num_points = 888
    sort_by = x
    execute_on = final
  []
  [./pre4_midplane]
    type = LineValueSampler
    variable = pre4
    start_point = '0.0390625 85 0'
    end_point = '69.3359375 85 0'
    num_points = 888
    sort_by = x
    execute_on = final
  []
  [./pre5_midplane]
    type = LineValueSampler
    variable = pre5
    start_point = '0.0390625 85 0'
    end_point = '69.3359375 85 0'
    num_points = 888
    sort_by = x
    execute_on = final
  []
  [./pre6_midplane]
    type = LineValueSampler
    variable = pre6
    start_point = '0.0390625 85 0'
    end_point = '69.3359375 85 0'
    num_points = 888
    sort_by = x
    execute_on = final
  []
[]

[MultiApps]
  [./ntsApp]
    type = FullSolveMultiApp
    app_type = MoltresApp
    execute_on = timestep_end
    positions = '0 0 0'
    input_files = 'msre-transient-nts-2nd.i'
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
  [../]
  [./csv]
    type = CSV
    execute_on = final
  [../]
[]

[Debug]
  show_var_residual_norms = true
[]
