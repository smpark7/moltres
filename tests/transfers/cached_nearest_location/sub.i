# 1-D source app placed in 3-D by constant displacements, like SAM's pipes: a line along z at
# (x, y) = (0.3, 0.6), split into two blocks
[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 1
    nx = 7
    elem_type = EDGE3
  []
  [lower]
    type = SubdomainBoundingBoxGenerator
    input = gen
    bottom_left = '0 -1 -1'
    top_right = '0.5 1 1'
    block_id = 1
  []
  displacements = 'disp_x disp_y disp_z'
[]

[AuxVariables]
  [disp_x]
    order = SECOND
  []
  [disp_y]
    order = SECOND
  []
  [disp_z]
    order = SECOND
  []
  [src]
    order = SECOND
  []
[]

[AuxKernels]
  [disp_x]
    type = FunctionAux
    variable = disp_x
    function = '0.3 - x'
    execute_on = initial
  []
  [disp_y]
    type = FunctionAux
    variable = disp_y
    function = 0.6
    execute_on = initial
  []
  [disp_z]
    type = FunctionAux
    variable = disp_z
    function = x
    execute_on = initial
  []
  [src]
    type = FunctionAux
    variable = src
    function = '1 + x + x * x + 10 * t'
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
