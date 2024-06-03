[GlobalParams]
  num_groups = 5
  num_precursor_groups = 6
  group_fluxes = 'group1 group2 group3 group4 group5'
  temperature = 900
  sss2_input = true
  account_delayed = false
  use_exp_form = false
[]

[Problem]
  type = EigenProblem
  bx_norm = bnorm
[]

[Mesh]
  second_order = true
  [cmg]
    type = CartesianMeshGenerator
    dim = 3
    dx = '0.25 0.24 0.01 1 1.5625 3.875 1.125 3.875 1.125 3.875 1.125 3.875 1.125 3.875 1.125 3.875 1.125 3.875 1.125 3.875 3.0625 15'
    ix = '5 8 10 5 10 20 6 20 6 20 6 20 6 20 6 20 6 20 6 20 14 75'
#    ix = '14 10 10 10 20 6 20 6 20 6 20 6 20 6 20 6 20 6 20 14 75'
#    ix = '10 10 20 40 12 40 12 40 12 40 12 40 12 40 12 40 12 40 28 150'
#    ix = '20 20 40 80 24 80 24 80 24 80 24 80 24 80 24 80 24 80 56 300'
    dy = '10'
    iy = '1'
    dz = '10'
    iz = '1'
    subdomain_id = '0 0 0 1 2 3 2 3 2 3 2 3 2 3 2 3 2 3 2 3 2 4'
  []
  [fuel]
    type = SubdomainBoundingBoxGenerator
    input = cmg
    restricted_subdomains = 2
    block_id = 5
    bottom_left = '0 0 0'
    top_right = '10 10 10'
  []
  [graphite]
    type = SubdomainBoundingBoxGenerator
    input = fuel
    restricted_subdomains = 3
    block_id = 6
    bottom_left = '0 0 0'
    top_right = '12.5 10 10'
  []
  [fuel_adaptive]
    type = SubdomainBoundingBoxGenerator
    input = graphite
    restricted_subdomains = 2
    block_id = 7
    bottom_left = '10 0 0'
    top_right = '15 10 10'
  []
[]

[AuxVariables]
  [drift1]
    order = SECOND
    family = MONOMIAL
    components = 3
  []
  [drift2]
    order = SECOND
    family = MONOMIAL
    components = 3
  []
  [drift3]
    order = SECOND
    family = MONOMIAL
    components = 3
  []
  [drift4]
    order = SECOND
    family = MONOMIAL
    components = 3
  []
  [drift5]
    order = SECOND
    family = MONOMIAL
    components = 3
  []
[]

[Nt]
  family = LAGRANGE
  order = SECOND
  var_name_base = group
  vacuum_boundaries = 'right'
  create_temperature_var = false
  eigen = true
[]

[Kernels]
  [group1_drift]
    type = GroupDrift
    variable = group1
    block = '0 1 5 6'
    group_drift_var = drift1
  []
  [group2_drift]
    type = GroupDrift
    variable = group2
    block = '0 1 5 6'
    group_drift_var = drift2
  []
  [group3_drift]
    type = GroupDrift
    variable = group3
    block = '0 1 5 6'
    group_drift_var = drift3
  []
  [group4_drift]
    type = GroupDrift
    variable = group4
    block = '0 1 5 6'
    group_drift_var = drift4
  []
  [group5_drift]
    type = GroupDrift
    variable = group5
    block = '0 1 5 6'
    group_drift_var = drift5
  []
  [group1_drift_adaptive]
    type = GroupDrift
    variable = group1
    block = '7'
    group_drift_var = drift1
    adaptive = false
  []
  [group2_drift_adaptive]
    type = GroupDrift
    variable = group2
    block = '7'
    group_drift_var = drift2
    adaptive = false
  []
  [group3_drift_adaptive]
    type = GroupDrift
    variable = group3
    block = '7'
    group_drift_var = drift3
    adaptive = false
  []
  [group4_drift_adaptive]
    type = GroupDrift
    variable = group4
    block = '7'
    group_drift_var = drift4
    adaptive = false
  []
  [group5_drift_adaptive]
    type = GroupDrift
    variable = group5
    block = '7'
    group_drift_var = drift5
    adaptive = false
  []
[]

[Materials]
  [absorber_sn]
    type = MoltresJsonMaterial
    base_file = 'openmc/absorber-air-lattice-ref-5g.json'
    material_key = 'ctrlrod'
    interp_type = 'none'
    block = '0'
  []
  [air]
    type = MoltresJsonMaterial
    base_file = 'openmc/absorber-air-lattice-ref-5g.json'
    material_key = 'air'
    interp_type = 'none'
    block = '1'
  []
  [fuel]
    type = MoltresJsonMaterial
    base_file = 'openmc/absorber-air-lattice-ref-5g.json'
    material_key = 'fuel'
    interp_type = 'none'
    block = '2 5 7'
  []
  [graphite]
    type = MoltresJsonMaterial
    base_file = 'openmc/absorber-air-lattice-ref-5g.json'
    material_key = 'graphite'
    interp_type = 'none'
    block = '3 6'
  []
  [ref]
    type = MoltresJsonMaterial
    base_file = 'openmc/absorber-air-lattice-ref-5g.json'
    material_key = 'reflector'
    interp_type = 'none'
    block = '4'
    temperature = 300
  []
[]

[Executioner]
  type = Eigenvalue
  free_power_iterations = 2

  fixed_point_abs_tol = 1e-12
  fixed_point_rel_tol = 1e-8
  fixed_point_max_its = 8
  accept_on_max_fixed_point_iteration = false
#  custom_pp = eigenvalue
#  custom_abs_tol = 1e-5

  nl_abs_tol = 1e-10
  nl_rel_tol = 1e-50

  automatic_scaling = true
  compute_scaling_once = true
  resid_vs_jac_scaling_param = 0.5

  solve_type = 'PJFNK'
  petsc_options = '-snes_converged_reason -ksp_converged_reason -snes_linesearch_monitor'
#  petsc_options_iname = '-pc_type -pc_hypre_type'
#  petsc_options_value = 'hypre boomeramg'
  petsc_options_iname = '-pc_type -pc_factor_shift_type -pc_factor_mat_solver_type'
  petsc_options_value = 'lu nonzero superlu_dist'
#  petsc_options_iname = '-pc_type -pc_hypre_type -pc_hypre_boomeramg_strong_threshold -pc_hypre_boomeramg_agg_nl -pc_hypre_boomeramg_agg_num_paths -pc_hypre_boomeramg_max_levels -pc_hypre_boomeramg_coarsen_type -pc_hypre_boomeramg_interp_type -pc_hypre_boomeramg_P_max -pc_hypre_boomeramg_truncfactor'
#  petsc_options_value = 'hypre boomeramg 0.7 4 5 25 HMIS ext+i 2 0.3'
  line_search = 'none'
[]

[MultiApps]
  [sub]
    type = FullSolveMultiApp
    input_files = absorber-air-lattice-ref-hybrid-5g-sub.i
    execute_on = timestep_end
    keep_solution_during_restore = true
  []
[]

[Transfers]
  [to_sub]
    type = MultiAppGeneralFieldShapeEvaluationTransfer
    source_variable = 'group1 group2 group3 group4 group5'
    variable = 'group1_diff group2_diff group3_diff group4_diff group5_diff'
    to_multi_app = sub
  []
  [to_sub_k]
    type = MultiAppPostprocessorTransfer
    from_postprocessor = eigenvalue
    to_postprocessor = eigenvalue
    to_multi_app = sub
  []
  [from_sub_drift1]
    type = MultiAppGeneralFieldShapeEvaluationTransfer
    source_variable = 'drift1 drift1 drift1'
    variable = 'drift1 drift1 drift1'
    from_multi_app = sub
    source_variable_components = '0 1 2'
    target_variable_components = '0 1 2'
  []
  [from_sub_drift2]
    type = MultiAppGeneralFieldShapeEvaluationTransfer
    source_variable = 'drift2 drift2 drift2'
    variable = 'drift2 drift2 drift2'
    from_multi_app = sub
    source_variable_components = '0 1 2'
    target_variable_components = '0 1 2'
  []
  [from_sub_drift3]
    type = MultiAppGeneralFieldShapeEvaluationTransfer
    source_variable = 'drift3 drift3 drift3'
    variable = 'drift3 drift3 drift3'
    from_multi_app = sub
    source_variable_components = '0 1 2'
    target_variable_components = '0 1 2'
  []
  [from_sub_drift4]
    type = MultiAppGeneralFieldShapeEvaluationTransfer
    source_variable = 'drift4 drift4 drift4'
    variable = 'drift4 drift4 drift4'
    from_multi_app = sub
    source_variable_components = '0 1 2'
    target_variable_components = '0 1 2'
  []
  [from_sub_drift5]
    type = MultiAppGeneralFieldShapeEvaluationTransfer
    source_variable = 'drift5 drift5 drift5'
    variable = 'drift5 drift5 drift5'
    from_multi_app = sub
    source_variable_components = '0 1 2'
    target_variable_components = '0 1 2'
  []
[]

[Postprocessors]
  [bnorm]
    type = ElmIntegTotFissNtsPostprocessor
    execute_on = 'initial linear timestep_end'
  []
  [eigenvalue]
    type = VectorPostprocessorComponent
    vectorpostprocessor = eigenvalues
    vector_name = eigen_values_real
    index = 0
  []
[]

[VectorPostprocessors]
  [eigenvalues]
    type = Eigenvalues
    inverse_eigenvalue = true
  []
[]

[Outputs]
  [exodus]
    type = Exodus
    discontinuous = true
    execute_on = 'initial timestep_end failed'
  []
[]
