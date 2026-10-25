# 3-D source app covering part of the parent domain, so that the transfers requiring the source
# app to contain the target point miss the parent nodes outside it
[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 3
    nx = 2
    ny = 2
    nz = 2
    xmax = 0.6
    ymax = 0.6
    zmax = 0.6
    elem_type = HEX27
  []
[]

[AuxVariables]
  [src]
    order = SECOND
  []
[]

[AuxKernels]
  [src]
    type = FunctionAux
    variable = src
    function = '1 + x + 2 * y + 3 * z + 10 * t'
    execute_on = 'initial timestep_end'
  []
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Transient
  num_steps = 3
[]
