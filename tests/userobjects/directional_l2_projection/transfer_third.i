# Third-order DirectionalL2Projection -> MultiAppGeneralFieldUserObjectTransfer -> LAGRANGE THIRD
# variable on a 1-D EDGE4 sub-app mesh whose elements are the projection layers.
#
# u(x,y,z) = 1 + 2 x - 0.5 x^2 + 0.25 x^3 + 3 y + 4 z^2 + 2 x y on [0,1.6] x [-0.05,0.05] x
# [-0.1,0.1]; its cross-section average is 1.0133333... + 2 x - 0.5 x^2 + 0.25 x^3, which the
# sub-app must receive exactly (err in the sub-app output at round-off). u is stored in a
# MONOMIAL THIRD variable, and SEVENTH-order quadrature (4 Gauss points per direction) gives
# each layer the p + 1 = 4 distinct positions it needs.

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
    expression = '1 + 2*x - 0.5*x*x + 0.25*x*x*x + 3*y + 4*z*z + 2*x*y'
  []
[]

[AuxVariables]
  [u]
    family = MONOMIAL
    order = THIRD
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
    order = THIRD
    num_layers = 4
    direction_min = 0
    direction_max = 1.6
    execute_on = 'initial'
  []
[]

[MultiApps]
  [channel]
    type = FullSolveMultiApp
    input_files = transfer_third_sub.i
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
  [Quadrature]
    order = SEVENTH
  []
[]
