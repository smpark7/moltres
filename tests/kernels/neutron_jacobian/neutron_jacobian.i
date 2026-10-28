# Jacobian check for the neutron source kernels, with temperature-dependent group constants and all
# variables coupled
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
    nx = 2
    ny = 2
    elem_type = QUAD9
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
    [InitialCondition]
      type = FunctionIC
      function = '0.2 + x'
    []
  []
  [pre3]
    order = SECOND
    [InitialCondition]
      type = FunctionIC
      function = '0.4 + y'
    []
  []
  [pre4]
    order = SECOND
    initial_condition = 0.5
  []
  [pre5]
    order = SECOND
    [InitialCondition]
      type = FunctionIC
      function = '0.1 + x * y'
    []
  []
  [pre6]
    order = SECOND
    initial_condition = 0.7
  []
  [temp]
    order = SECOND
    [InitialCondition]
      type = FunctionIC
      function = '940 + 40 * x + 30 * y'
    []
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
  [heat]
    type = FissionHeatSource
    variable = temp
    tot_fission_heat = 0.02
    power = 3
  []
[]

[Materials]
  [fuel]
    type = GenericMoltresMaterial
    property_tables_root = '../../../property_file_dir/newt_msre_converted_to_serpent/serpent_msre_fuel_'
    interp_type = 'spline'
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
  num_steps = 1
  dt = 1e-3
  solve_type = NEWTON
[]
