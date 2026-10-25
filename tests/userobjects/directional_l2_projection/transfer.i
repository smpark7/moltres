# DirectionalL2Projection -> MultiAppGeneralFieldUserObjectTransfer -> LAGRANGE SECOND variable on a
# 1-D EDGE3 sub-app mesh whose elements are the projection layers (as for a SAM flow channel).
#
# u(x,y,z) = 1 + 2 x - 0.5 x^2 + 3 y + 4 z^2 + 2 x y on [0,1.6] x [-0.05,0.05] x [-0.1,0.1];
# its cross-section average is 1.0133333... + 2 x - 0.5 x^2, which the sub-app must receive
# exactly (err in the sub-app output at round-off).

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 3
    nx = 4
    ny = 1
    nz = 2
    xmax = 1.6
    ymin = -0.05
    ymax = 0.05
    zmin = -0.1
    zmax = 0.1
    elem_type = HEX27
  []
[]

[Variables]
  [dummy]
    family = LAGRANGE
    order = SECOND
  []
[]

[Kernels]
  [diff]
    type = Diffusion
    variable = dummy
  []
[]

[Functions]
  [u_func]
    type = ParsedFunction
    expression = '1 + 2*x - 0.5*x*x + 3*y + 4*z*z + 2*x*y'
  []
[]

[AuxVariables]
  [u]
    family = L2_LAGRANGE
    order = SECOND
  []
[]

[AuxKernels]
  [u_aux]
    type = FunctionAux
    variable = u
    function = u_func
    execute_on = 'initial'
  []
[]

[UserObjects]
  [projection]
    type = DirectionalL2Projection
    variable = u
    direction = x
    order = SECOND
    num_layers = 4
    direction_min = 0
    direction_max = 1.6
    execute_on = 'initial'
  []
[]

[MultiApps]
  [channel]
    type = FullSolveMultiApp
    input_files = transfer_sub.i
    execute_on = 'timestep_end'
  []
[]

[Transfers]
  [to_channel]
    type = MultiAppGeneralFieldUserObjectTransfer
    to_multi_app = channel
    source_user_object = projection
    variable = received
    execute_on = 'timestep_end'
  []
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Steady
[]
