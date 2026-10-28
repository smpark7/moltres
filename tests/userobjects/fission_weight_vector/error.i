# FissionWeightVector must refuse cross sections that depend on a solver variable
[GlobalParams]
  num_groups = 2
  num_precursor_groups = 6
  group_fluxes = 'group1 group2'
  temperature = temp
  sss2_input = true
[]

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 2
    ny = 2
  []
[]

[Variables]
  [group1]
  []
  [group2]
  []
  [temp]
    initial_condition = 950
  []
[]

[Kernels]
  [diff1]
    type = Diffusion
    variable = group1
  []
  [diff2]
    type = Diffusion
    variable = group2
  []
  [difft]
    type = Diffusion
    variable = temp
  []
[]

[Materials]
  [fuel]
    type = GenericMoltresMaterial
    property_tables_root = '../../../property_file_dir/newt_msre_converted_to_serpent/serpent_msre_fuel_'
    interp_type = 'linear'
  []
[]

[UserObjects]
  [weights]
    type = FissionWeightVector
    quantity = fission_neutrons
  []
[]

[Executioner]
  type = Steady
[]
