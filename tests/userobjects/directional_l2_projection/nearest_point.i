# NearestPointDirectionalL2Projection / NearestPointDirectionalSideL2Projection: two channels side by side
# (x in [0, 0.2] and [0.2, 0.4]), one point each.
#
# u(x,y,z) = (1 + 2 z - 0.5 z^2) (1 + 3 x) + 4 y^2, exact in L2_LAGRANGE SECOND on HEX27.
# Cross-section averages over each channel:
#   volume            : 1.3 (1 + 2 z - 0.5 z^2) + 0.01333... (channel 1), 1.9 (...) + 0.01333... (2)
#   side 'bottom' (y=0): 1.3 (1 + 2 z - 0.5 z^2)                (channel 1), 1.9 (...)            (2)
# The nodal values of both projections (channel 1 then channel 2, 9 nodes each along z) are
# written by SpatialUserObjectVectorPostprocessor and must equal these averages at the nodes.

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 3
    nx = 4
    ny = 1
    nz = 4
    xmax = 0.4
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
    expression = '(1 + 2*z - 0.5*z*z) * (1 + 3*x) + 4*y*y'
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
    type = NearestPointDirectionalL2Projection
    variable = u
    points = '0.1 0.05 0  0.3 0.05 0'
    direction = z
    order = SECOND
    bounds = '0 0.4 0.8 1.2 1.6'
    execute_on = 'initial'
  []
  [side_projection]
    type = NearestPointDirectionalSideL2Projection
    variable = u
    boundary = bottom
    points = '0.1 0 0  0.3 0 0'
    direction = z
    order = SECOND
    num_layers = 4
    direction_min = 0
    direction_max = 1.6
    execute_on = 'initial'
  []
[]

[VectorPostprocessors]
  [nodal]
    type = SpatialUserObjectVectorPostprocessor
    userobject = projection
    execute_on = 'timestep_end'
  []
  [side_nodal]
    type = SpatialUserObjectVectorPostprocessor
    userobject = side_projection
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
