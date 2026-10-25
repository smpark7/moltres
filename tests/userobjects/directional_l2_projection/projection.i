# DirectionalL2Projection / DirectionalSideL2Projection of a quadratic field.
#
# u(x,y,z) = 1 + 2 z - 0.5 z^2 + 3 x + 4 y^2 + 2 x z on [0,0.2] x [0,0.1] x [0,1.6], stored in an
# L2_LAGRANGE SECOND variable (exact for this quadratic on HEX27 and PRISM18).
# Its cross-section average is quadratic in z,
#   volume       : 1.31333... + 2.2 z - 0.5 z^2
#   side 'left'  : 1.01333... + 2.0 z - 0.5 z^2   (x = 0, averaged over y)
# so the projection onto the C0 P2 field on the 4 layers must reproduce it exactly. The
# projection is sampled at the nodes of a LAGRANGE SECOND variable (as a transfer would) and
# compared with the exact average: err must be at round-off.

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 3
    nx = 2
    ny = 1
    nz = 4
    xmax = 0.2
    ymax = 0.1
    zmax = 1.6
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
    expression = '1 + 2*z - 0.5*z*z + 3*x + 4*y*y + 2*x*z'
  []
  [average]
    type = ParsedFunction
    expression = '1.3133333333333333 + 2.2*z - 0.5*z*z'
  []
[]

[AuxVariables]
  [u]
    family = L2_LAGRANGE
    order = SECOND
  []
  [projected]
    family = LAGRANGE
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
  [projected_aux]
    type = FunctorAux
    variable = projected
    functor = projection
    execute_on = 'timestep_end'
  []
[]

[UserObjects]
  [projection]
    type = DirectionalL2Projection
    variable = u
    direction = z
    order = SECOND
    num_layers = 4
    direction_min = 0
    direction_max = 1.6
    execute_on = 'initial'
  []
[]

[Postprocessors]
  [err]
    type = ElementL2Error
    variable = projected
    function = average
    execute_on = 'timestep_end'
  []
[]

[VectorPostprocessors]
  [nodal]
    type = SpatialUserObjectVectorPostprocessor
    userobject = projection
    execute_on = 'timestep_end'
  []
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Steady
[]

[Outputs]
  csv = true
  execute_on = 'timestep_end'
[]
