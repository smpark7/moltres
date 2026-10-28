# W . u from FissionWeightVector + WeightVectorDotProduct against the element-integral fission
# postprocessors, with cross sections that follow a time-dependent auxiliary temperature
[GlobalParams]
  num_groups = 2
  num_precursor_groups = 6
  group_fluxes = 'group1 group2'
  pre_concs = 'pre1 pre2 pre3 pre4 pre5 pre6'
  temperature = temp
  use_exp_form = false
  account_delayed = true
  sss2_input = true
[]

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 6
    ny = 6
    elem_type = QUAD9
  []
  [fuel]
    type = SubdomainBoundingBoxGenerator
    input = gen
    bottom_left = '0 0 0'
    top_right = '0.5 1 0'
    block_id = 1
  []
[]

[Variables]
  [group1]
    order = SECOND
    [InitialCondition]
      type = FunctionIC
      function = '1 + x + 0.5 * y'
    []
  []
  [group2]
    order = SECOND
    [InitialCondition]
      type = FunctionIC
      function = '0.5 + x * y + 0.3 * x'
    []
  []
  [pre1]
    order = SECOND
    initial_condition = 0.3
  []
  [pre2]
    order = SECOND
    initial_condition = 0.2
  []
  [pre3]
    order = SECOND
    initial_condition = 0.4
  []
  [pre4]
    order = SECOND
    initial_condition = 0.5
  []
  [pre5]
    order = SECOND
    initial_condition = 0.1
  []
  [pre6]
    order = SECOND
    initial_condition = 0.7
  []
[]

[AuxVariables]
  [temp]
    order = SECOND
  []
[]

[AuxKernels]
  [temp]
    type = FunctionAux
    variable = temp
    function = '930 + 30 * x + 20 * y + 1e4 * t'
    execute_on = 'initial timestep_begin'
  []
[]

[Kernels]
  [time1]
    type = NtTimeDerivative
    variable = group1
    group_number = 1
  []
  [time2]
    type = NtTimeDerivative
    variable = group2
    group_number = 2
  []
  [diffusion1]
    type = GroupDiffusion
    variable = group1
    group_number = 1
  []
  [diffusion2]
    type = GroupDiffusion
    variable = group2
    group_number = 2
  []
  [sigma_r1]
    type = SigmaR
    variable = group1
    group_number = 1
  []
  [sigma_r2]
    type = SigmaR
    variable = group2
    group_number = 2
  []
  [fission1]
    type = CoupledFissionKernel
    variable = group1
    group_number = 1
  []
  [fission2]
    type = CoupledFissionKernel
    variable = group2
    group_number = 2
  []
  [inscatter1]
    type = InScatter
    variable = group1
    group_number = 1
  []
  [inscatter2]
    type = InScatter
    variable = group2
    group_number = 2
  []
  [delayed1]
    type = DelayedNeutronSource
    variable = group1
    group_number = 1
  []
  [delayed2]
    type = DelayedNeutronSource
    variable = group2
    group_number = 2
  []
  [pre1_source]
    type = PrecursorSource
    variable = pre1
    precursor_group_number = 1
  []
  [pre2_source]
    type = PrecursorSource
    variable = pre2
    precursor_group_number = 2
  []
  [pre3_source]
    type = PrecursorSource
    variable = pre3
    precursor_group_number = 3
  []
  [pre4_source]
    type = PrecursorSource
    variable = pre4
    precursor_group_number = 4
  []
  [pre5_source]
    type = PrecursorSource
    variable = pre5
    precursor_group_number = 5
  []
  [pre6_source]
    type = PrecursorSource
    variable = pre6
    precursor_group_number = 6
  []
  [pre_time1]
    type = TimeDerivative
    variable = pre1
  []
  [pre_time2]
    type = TimeDerivative
    variable = pre2
  []
  [pre_time3]
    type = TimeDerivative
    variable = pre3
  []
  [pre_time4]
    type = TimeDerivative
    variable = pre4
  []
  [pre_time5]
    type = TimeDerivative
    variable = pre5
  []
  [pre_time6]
    type = TimeDerivative
    variable = pre6
  []
[]

[Materials]
  [fuel]
    type = GenericMoltresMaterial
    property_tables_root = '../../../property_file_dir/newt_msre_converted_to_serpent/serpent_msre_fuel_'
    interp_type = 'spline'
    block = 1
  []
  [moderator]
    type = GenericMoltresMaterial
    property_tables_root = '../../../property_file_dir/newt_msre_converted_to_serpent/serpent_msre_mod_'
    interp_type = 'spline'
    block = 0
  []
[]

[UserObjects]
  [heat_weights]
    type = FissionWeightVector
    quantity = fission_heat
    nt_scale = 1e3
    block = 1
  []
  [neutron_weights]
    type = FissionWeightVector
    quantity = fission_neutrons
  []
[]

[Postprocessors]
  [heat]
    type = ElmIntegTotFissHeatPostprocessor
    nt_scale = 1e3
    block = 1
    execute_on = 'linear timestep_end'
  []
  [heat_wu]
    type = WeightVectorDotProduct
    weight_vector = heat_weights
    execute_on = 'linear timestep_end'
  []
  [neutrons]
    type = ElmIntegTotFissNtsPostprocessor
    account_delayed = false
    execute_on = 'linear timestep_end'
  []
  [neutrons_wu]
    type = WeightVectorDotProduct
    weight_vector = neutron_weights
    execute_on = 'linear timestep_end'
  []
  [heat_rel_diff]
    type = ParsedPostprocessor
    expression = 'abs(heat_wu - heat) / heat'
    pp_names = 'heat heat_wu'
    execute_on = 'linear timestep_end'
  []
  [neutrons_rel_diff]
    type = ParsedPostprocessor
    expression = 'abs(neutrons_wu - neutrons) / neutrons'
    pp_names = 'neutrons neutrons_wu'
    execute_on = 'linear timestep_end'
  []
[]

[Preconditioning]
  [smp]
    type = SMP
    full = true
  []
[]

[Executioner]
  type = Transient
  num_steps = 3
  dt = 1e-3
  solve_type = NEWTON
  petsc_options_iname = '-pc_type -pc_factor_mat_solver_type'
  petsc_options_value = 'lu mumps'
  nl_rel_tol = 1e-8
[]

[Outputs]
  csv = true
[]
