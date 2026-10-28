# Samples a multi-dimensional group-constant table (xsdata-2d.json) over a range of fuel and
# moderator temperatures and compares the group constants and their derivatives with the closed
# form the table was built from (make_xsdata_2d.py): every constant is its 900 K value in
# xsdata.json times a factor that is bilinear in the two temperatures, which multilinear
# interpolation reproduces exactly. Other tables in the file add quadratic terms (qf, qm below),
# reproduced exactly by a quadratic least-squares fit, or cubic terms, for which the tests file
# gives the expected factor from a reference fit. The material type and the source of the other
# temperature are set on the command line (see the tests file).
#
# Triangles with one quadrature point at the centroid keep every field exact at the point where
# the material is evaluated (linear temperature fields, constant-monomial copies of the
# properties).

# Temperature ranges sampled along x (fuel) and y (moderator); the table covers 900-1200 K and
# 800-1000 K
tf_min = 900
tf_max = 1200
tm_min = 800
tm_max = 1000

# 1 to compare with the table clamped to its range
clamp = 0

# Expected factor and its derivative along the material's own axis, in terms of the fuel (tfe) and
# moderator (tme) temperatures that the table sees
qf = 0
qm = 0
factor = '1 + 2e-4 * (tfe - 1000) - 3e-4 * (tme - 900) + 1e-6 * (tfe - 1000) * (tme - 900) + qf * (tfe - 1000)^2 + qm * (tme - 900)^2'
dfactor = '2e-4 + 1e-6 * (tme - 900) + 2 * qf * (tfe - 1000)'

# Copy of the moderator temperature the material is given (tmod_fn: tm_fn, tmod_uo: tm_avg)
tmod = tmod_fn

# 900 K values in xsdata.json
remxs0 = 0.006341844392102427
nsf1 = 0.04586728338630509
gtransfxs1 = 0.002276926569587877
beta0 = 0.006511824819295404

[GlobalParams]
  num_groups = 2
  num_precursor_groups = 6
[]

[Mesh]
  [mesh]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 12
    ny = 12
    elem_type = TRI3
  []
[]

[Functions]
  [tm_fn]
    type = ParsedFunction
    expression = '${tm_min} + (${tm_max} - ${tm_min}) * y'
  []
[]

[UserObjects]
  [tm_avg]
    type = LayeredAverage
    variable = tm
    direction = y
    num_layers = 5
    execute_on = 'initial timestep_begin'
  []
[]

[Materials]
  [fuel]
    type = MoltresJsonMaterial
    base_file = xsdata-2d.json
    material_key = fuel
    interp_type = linear
    temperature = tf
  []
[]

# A trivial nonlinear solve, so that the derivative properties are evaluated during Jacobian assembly
[Variables]
  [u]
  []
[]

[Kernels]
  [diff]
    type = Diffusion
    variable = u
  []
[]

[BCs]
  [left]
    type = DirichletBC
    variable = u
    boundary = left
    value = 0
  []
  [right]
    type = DirichletBC
    variable = u
    boundary = right
    value = 1
  []
[]

[AuxVariables]
  [tf]
  []
  [tm]
  []
  [tmod_fn]
    family = MONOMIAL
    order = CONSTANT
  []
  [tmod_uo]
    family = MONOMIAL
    order = CONSTANT
  []
  [tfe]
    family = MONOMIAL
    order = CONSTANT
  []
  [tme]
    family = MONOMIAL
    order = CONSTANT
  []
  [remxs0]
    family = MONOMIAL
    order = CONSTANT
  []
  [nsf1]
    family = MONOMIAL
    order = CONSTANT
  []
  [gtransfxs1]
    family = MONOMIAL
    order = CONSTANT
  []
  [beta]
    family = MONOMIAL
    order = CONSTANT
  []
  [d_remxs0]
    family = MONOMIAL
    order = CONSTANT
  []
  [err_remxs0]
    family = MONOMIAL
    order = CONSTANT
  []
  [err_nsf1]
    family = MONOMIAL
    order = CONSTANT
  []
  [err_gtransfxs1]
    family = MONOMIAL
    order = CONSTANT
  []
  [err_beta]
    family = MONOMIAL
    order = CONSTANT
  []
  [err_d_remxs0]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[ICs]
  [tf]
    type = FunctionIC
    variable = tf
    function = '${tf_min} + (${tf_max} - ${tf_min}) * x'
  []
  [tm]
    type = FunctionIC
    variable = tm
    function = tm_fn
  []
[]

[AuxKernels]
  [tmod_fn]
    type = FunctionAux
    variable = tmod_fn
    function = tm_fn
    execute_on = 'initial timestep_begin'
  []
  [tmod_uo]
    type = SpatialUserObjectAux
    variable = tmod_uo
    user_object = tm_avg
    execute_on = 'initial timestep_begin'
  []
  [tfe]
    type = ParsedAux
    variable = tfe
    coupled_variables = tf
    expression = 'if(${clamp} > 0, min(max(tf, 900), 1200), tf)'
    execute_on = 'initial timestep_begin'
  []
  [tme]
    type = ParsedAux
    variable = tme
    coupled_variables = ${tmod}
    expression = 'if(${clamp} > 0, min(max(${tmod}, 800), 1000), ${tmod})'
    execute_on = 'initial timestep_begin'
  []
  [remxs0]
    type = MaterialStdVectorAux
    variable = remxs0
    property = remxs
    index = 0
  []
  [nsf1]
    type = MaterialStdVectorAux
    variable = nsf1
    property = nsf
    index = 1
  []
  [gtransfxs1]
    type = MaterialStdVectorAux
    variable = gtransfxs1
    property = gtransfxs
    index = 1
  []
  [beta]
    type = MaterialRealAux
    variable = beta
    property = beta
  []
  # Derivatives are only computed during Jacobian assembly
  [d_remxs0]
    type = MaterialStdVectorAux
    variable = d_remxs0
    property = d_remxs_d_temp
    index = 0
    execute_on = nonlinear
  []
  [err_remxs0]
    type = ParsedAux
    variable = err_remxs0
    coupled_variables = 'remxs0 tfe tme'
    expression = 'abs(remxs0 / (${remxs0} * (${factor})) - 1)'
    constant_names = 'qf qm'
    constant_expressions = '${qf} ${qm}'
  []
  [err_nsf1]
    type = ParsedAux
    variable = err_nsf1
    coupled_variables = 'nsf1 tfe tme'
    expression = 'abs(nsf1 / (${nsf1} * (${factor})) - 1)'
    constant_names = 'qf qm'
    constant_expressions = '${qf} ${qm}'
  []
  [err_gtransfxs1]
    type = ParsedAux
    variable = err_gtransfxs1
    coupled_variables = 'gtransfxs1 tfe tme'
    expression = 'abs(gtransfxs1 / (${gtransfxs1} * (${factor})) - 1)'
    constant_names = 'qf qm'
    constant_expressions = '${qf} ${qm}'
  []
  [err_beta]
    type = ParsedAux
    variable = err_beta
    coupled_variables = 'beta tfe tme'
    expression = 'abs(beta / (${beta0} * (${factor})) - 1)'
    constant_names = 'qf qm'
    constant_expressions = '${qf} ${qm}'
  []
  # Relative to the smallest slope in the table
  [err_d_remxs0]
    type = ParsedAux
    variable = err_d_remxs0
    coupled_variables = 'd_remxs0 tf tfe tme'
    expression = 'abs(d_remxs0 - ${remxs0} * (${dfactor})) / (${remxs0} * 1e-4)'
    constant_names = 'qf qm'
    constant_expressions = '${qf} ${qm}'
    execute_on = nonlinear
  []
[]

[Postprocessors]
  [err_remxs0]
    type = ElementExtremeValue
    variable = err_remxs0
  []
  [err_nsf1]
    type = ElementExtremeValue
    variable = err_nsf1
  []
  [err_gtransfxs1]
    type = ElementExtremeValue
    variable = err_gtransfxs1
  []
  [err_beta]
    type = ElementExtremeValue
    variable = err_beta
  []
  [err_d_remxs0]
    type = ElementExtremeValue
    variable = err_d_remxs0
  []
  # Guards against a vacuous comparison
  [min_remxs0]
    type = ElementExtremeValue
    variable = remxs0
    value_type = min
  []
  [max_remxs0]
    type = ElementExtremeValue
    variable = remxs0
  []
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
  [Quadrature]
    order = CONSTANT
  []
[]

[Outputs]
  csv = true
  execute_on = final
[]
