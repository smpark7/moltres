# nts.i with bx_norm and normalization from W . u (FissionWeightVector + WeightVectorDotProduct);
# k_eff must match nts.i
[GlobalParams]
  num_groups = 2
  num_precursor_groups = 6
  use_exp_form = false
  group_fluxes = 'group1 group2'
  temperature = 922
  sss2_input = false
  account_delayed = false
[]

[Problem]
  type = EigenProblem
  bx_norm = fiss_neutrons_wu
[]

[Mesh]
  coord_type = RZ
  file = '../../nts/2d_lattice_structured_smaller.msh'
[]

[Nt]
  var_name_base = group
  vacuum_boundaries = 'fuel_bottoms fuel_tops moder_bottoms moder_tops outer_wall'
  create_temperature_var = false
  eigen = true
[]

[Materials]
  [fuel]
    type = GenericMoltresMaterial
    property_tables_root = '../../../property_file_dir/newt_msre_fuel_'
    interp_type = 'spline'
    block = 'fuel'
  []
  [moder]
    type = GenericMoltresMaterial
    property_tables_root = '../../../property_file_dir/newt_msre_mod_'
    interp_type = 'spline'
    block = 'moder'
  []
[]

[Executioner]
  type = Eigenvalue
  eigen_tol = 1e-6
  free_power_iterations = 2
  normalization = fiss_neutrons_wu
  normal_factor = 1
  solve_type = 'PJFNK'
  petsc_options = '-snes_converged_reason -ksp_converged_reason -snes_linesearch_monitor'
  petsc_options_iname = '-pc_type -sub_pc_type'
  petsc_options_value = 'asm lu'
[]

[Preconditioning]
  [SMP]
    type = SMP
    full = true
  []
[]

[Postprocessors]
  [k_eff]
    type = VectorPostprocessorComponent
    index = 0
    vectorpostprocessor = k_vpp
    vector_name = eigen_values_real
  []
  [fiss_neutrons]
    type = ElmIntegTotFissNtsPostprocessor
    execute_on = 'linear timestep_end'
  []
  [fiss_neutrons_wu]
    type = WeightVectorDotProduct
    weight_vector = neutron_weights
    execute_on = linear
  []
  [tot_fiss]
    type = ElmIntegTotFissPostprocessor
    execute_on = linear
  []
  [group1norm]
    type = ElementIntegralVariablePostprocessor
    variable = group1
  []
  [group2norm]
    type = ElementIntegralVariablePostprocessor
    variable = group2
  []
[]

[UserObjects]
  [neutron_weights]
    type = FissionWeightVector
    quantity = fission_neutrons
  []
[]

[VectorPostprocessors]
  [k_vpp]
    type = Eigenvalues
    inverse_eigenvalue = true
    contains_complete_history = true
  []
[]

[Outputs]
  csv = true
  execute_on = final
[]
