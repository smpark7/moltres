N = 8
L = 2
total_ords = ${fparse N*(N+2)}

[GlobalParams]
  num_groups = 5
  num_precursor_groups = 6
  group_angular_fluxes = 'psi1 psi2 psi3 psi4 psi5'
  temperature = 900
  N = ${N}
  L = ${L}
  account_delayed = false
  use_exp_form = false
  group_constants = 'TOTXS FISSXS NSF FISSE RECIPVEL CHI_T CHI_P CHI_D SPN BETA_EFF DECAY_CONSTANT DIFFCOEF'
[]

[Problem]
  type = EigenProblem
  bx_norm = nts
[]

[Mesh]
  second_order = true
  [cmg]
    type = CartesianMeshGenerator
    dim = 3
    dx = '0.49 0.01 1 1.5625 3.875 1.125 3.875 1.125 3.875 1.125 3.875 1.125 3.875 1.125 3.875 1.125 3.875 1.125 3.875 3.0625 15'
    ix = '7 10 5 10 20 6 20 6 20 6 20 6 20 6 20 6 20 6 20 14 75'
#    ix = '20 10 5 10 3 10 3 10 3 10 3 10 3 10 3 10 3 10 7 30'
    dy = '10'
    iy = '1'
    dz = '10'
    iz = '1'
    subdomain_id = '0 0 1 2 3 2 3 2 3 2 3 2 3 2 3 2 3 2 3 2 4'
  []
[]

[Variables]
  [psi1]
    order = SECOND
    family = LAGRANGE
    components = ${total_ords}
  []
  [psi2]
    order = SECOND
    family = LAGRANGE
    components = ${total_ords}
  []
  [psi3]
    order = SECOND
    family = LAGRANGE
    components = ${total_ords}
  []
  [psi4]
    order = SECOND
    family = LAGRANGE
    components = ${total_ords}
  []
  [psi5]
    order = SECOND
    family = LAGRANGE
    components = ${total_ords}
  []
[]

[AuxVariables]
  [group1]
    order = SECOND
    family = LAGRANGE
  []
  [group2]
    order = SECOND
    family = LAGRANGE
  []
  [group3]
    order = SECOND
    family = LAGRANGE
  []
  [group4]
    order = SECOND
    family = LAGRANGE
  []
  [group5]
    order = SECOND
    family = LAGRANGE
  []
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

[Kernels]
  [streaming_psi1]
    type = SNStreaming
    variable = psi1
    group_number = 1
  []
  [streaming_psi2]
    type = SNStreaming
    variable = psi2
    group_number = 2
  []
  [streaming_psi3]
    type = SNStreaming
    variable = psi3
    group_number = 3
  []
  [streaming_psi4]
    type = SNStreaming
    variable = psi4
    group_number = 4
  []
  [streaming_psi5]
    type = SNStreaming
    variable = psi5
    group_number = 5
  []
  [collision_psi1]
    type = SNCollision
    variable = psi1
    group_number = 1
  []
  [collision_psi2]
    type = SNCollision
    variable = psi2
    group_number = 2
  []
  [collision_psi3]
    type = SNCollision
    variable = psi3
    group_number = 3
  []
  [collision_psi4]
    type = SNCollision
    variable = psi4
    group_number = 4
  []
  [collision_psi5]
    type = SNCollision
    variable = psi5
    group_number = 5
  []
  [scattering_psi1]
    type = SNScattering
    variable = psi1
    group_number = 1
  []
  [scattering_psi2]
    type = SNScattering
    variable = psi2
    group_number = 2
  []
  [scattering_psi3]
    type = SNScattering
    variable = psi3
    group_number = 3
  []
  [scattering_psi4]
    type = SNScattering
    variable = psi4
    group_number = 4
  []
  [scattering_psi5]
    type = SNScattering
    variable = psi5
    group_number = 5
  []
  [fission_psi1]
    type = SNFission
    variable = psi1
    group_number = 1
    extra_vector_tags = 'eigen'
  []
  [fission_psi2]
    type = SNFission
    variable = psi2
    group_number = 2
    extra_vector_tags = 'eigen'
  []
  [fission_psi3]
    type = SNFission
    variable = psi3
    group_number = 3
    extra_vector_tags = 'eigen'
  []
  [fission_psi4]
    type = SNFission
    variable = psi4
    group_number = 4
    extra_vector_tags = 'eigen'
  []
  [fission_psi5]
    type = SNFission
    variable = psi5
    group_number = 5
    extra_vector_tags = 'eigen'
  []
[]

[AuxKernels]
  [scalar_group1]
    type = SNScalarFluxAux
    variable = group1
    psi = psi1
  []
  [scalar_group2]
    type = SNScalarFluxAux
    variable = group2
    psi = psi2
  []
  [scalar_goup3]
    type = SNScalarFluxAux
    variable = group3
    psi = psi3
  []
  [scalar_group4]
    type = SNScalarFluxAux
    variable = group4
    psi = psi4
  []
  [scalar_group5]
    type = SNScalarFluxAux
    variable = group5
    psi = psi5
  []
  [drift1]
    type = GroupDriftAux
    variable = drift1
    group_number = 1
  []
  [drift2]
    type = GroupDriftAux
    variable = drift2
    group_number = 2
  []
  [drift3]
    type = GroupDriftAux
    variable = drift3
    group_number = 3
  []
  [drift4]
    type = GroupDriftAux
    variable = drift4
    group_number = 4
  []
  [drift5]
    type = GroupDriftAux
    variable = drift5
    group_number = 5
  []
[]

[BCs]
  [vacuum_psi1]
    type = SNVacuumBC
    variable = psi1
    boundary = 'right'
  []
  [vacuum_psi2]
    type = SNVacuumBC
    variable = psi2
    boundary = 'right'
  []
  [vacuum_psi3]
    type = SNVacuumBC
    variable = psi3
    boundary = 'right'
  []
  [vacuum_psi4]
    type = SNVacuumBC
    variable = psi4
    boundary = 'right'
  []
  [vacuum_psi5]
    type = SNVacuumBC
    variable = psi5
    boundary = 'right'
  []
  [reflecting_psi1]
    type = SNReflectingBC
    variable = psi1
    boundary = 'front back bottom top left'
  []
  [reflecting_psi2]
    type = SNReflectingBC
    variable = psi2
    boundary = 'front back bottom top left'
  []
  [reflecting_psi3]
    type = SNReflectingBC
    variable = psi3
    boundary = 'front back bottom top left'
  []
  [reflecting_psi4]
    type = SNReflectingBC
    variable = psi4
    boundary = 'front back bottom top left'
  []
  [reflecting_psi5]
    type = SNReflectingBC
    variable = psi5
    boundary = 'front back bottom top left'
  []
[]

[Materials]
  [absorber]
    type = MoltresSNMaterial
    base_file = 'openmc/absorber-air-lattice-ref-5g.json'
    material_key = 'ctrlrod'
    interp_type = 'none'
    block = '0'
  []
  [air]
    type = MoltresSNMaterial
    base_file = 'openmc/absorber-air-lattice-ref-5g.json'
    material_key = 'air'
    interp_type = 'none'
    block = '1'
    void_constant = .5
    h_type = min
  []
  [fuel]
    type = MoltresSNMaterial
    base_file = 'openmc/absorber-air-lattice-ref-5g.json'
    material_key = 'fuel'
    interp_type = 'none'
    block = '2'
  []
  [graphite]
    type = MoltresSNMaterial
    base_file = 'openmc/absorber-air-lattice-ref-5g.json'
    material_key = 'graphite'
    interp_type = 'none'
    block = '3'
  []
  [ref]
    type = MoltresSNMaterial
    base_file = 'openmc/absorber-air-lattice-ref-5g.json'
    material_key = 'reflector'
    interp_type = 'none'
    block = '4'
    temperature = 300
  []
[]

[Executioner]
  type = Eigenvalue
  initial_eigenvalue = 1.4
  eigen_tol = 1e-7
  free_power_iterations = 2

  solve_type = 'PJFNK'
  petsc_options = '-snes_converged_reason -ksp_converged_reason -snes_linesearch_monitor'
  petsc_options_iname = '-pc_type -pc_hypre_type -pc_hypre_boomeramg_strong_threshold -pc_hypre_boomeramg_agg_nl -pc_hypre_boomeramg_agg_num_paths -pc_hypre_boomeramg_max_levels -pc_hypre_boomeramg_coarsen_type -pc_hypre_boomeramg_interp_type -pc_hypre_boomeramg_P_max -pc_hypre_boomeramg_truncfactor'
  petsc_options_value = 'hypre boomeramg 0.7 4 5 25 HMIS ext+i 2 0.3'
#  petsc_options_iname = '-pc_type -pc_hypre_type'
#  petsc_options_value = 'hypre boomeramg'
  line_search = 'none'

  automatic_scaling = true
  compute_scaling_once = true
  resid_vs_jac_scaling_param = 0.5

#  normal_factor = 1
#  normalization = nts

  nl_abs_tol = 1e-10
  nl_rel_tol = 1e-50
  l_abs_tol = 9e-11
#  nl_max_its = 20
#  l_max_its = 200
#  l_tol = 1e-2
[]

[Postprocessors]
  [nts]
    type = ElmIntegTotFissNtsPostprocessor
    group_fluxes = 'group1 group2 group3 group4 group5'
    execute_on = linear
  []
  [fission_rate]
    type = ElmIntegTotFissPostprocessor
    group_fluxes = 'group1 group2 group3 group4 group5'
    execute_on = linear
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
  perf_graph = true
  print_linear_residuals = true
  exodus = true
  csv = true
[]

[Debug]
#  show_var_residual_norms = true
[]
