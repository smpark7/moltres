# Each pair of target variables receives the same field from the 1-D sub-app, once with the stock
# nearest-location transfer (ref_*) and once with the cached one (cached_*). The differences
# must be exactly zero at every step, while the transferred field changes every step. The missed
# pair takes a field from a 3-D app covering part of the domain and requires the source app to
# contain the target point, so the nodes outside it are missed and keep their initial value.
[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 3
    nx = 3
    ny = 3
    nz = 5
    elem_type = HEX27
  []
  [right]
    type = SubdomainBoundingBoxGenerator
    input = gen
    bottom_left = '0.5 -1 -1'
    top_right = '2 2 2'
    block_id = 1
  []
[]

[AuxVariables]
  [ref_all]
    order = SECOND
  []
  [cached_all]
    order = SECOND
  []
  [ref_block]
    order = SECOND
    block = 1
  []
  [cached_block]
    order = SECOND
    block = 1
  []
  [ref_boundary]
    order = SECOND
  []
  [cached_boundary]
    order = SECOND
  []
  [ref_from_block]
    order = SECOND
  []
  [cached_from_block]
    order = SECOND
  []
  [ref_elem]
    family = MONOMIAL
    order = CONSTANT
  []
  [cached_elem]
    family = MONOMIAL
    order = CONSTANT
  []
  [ref_missed]
    order = SECOND
  []
  [cached_missed]
    order = SECOND
  []
  [diff_nodal]
    order = SECOND
  []
  [diff_block]
    order = SECOND
    block = 1
  []
  [diff_elem]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[AuxKernels]
  [diff_nodal]
    type = ParsedAux
    variable = diff_nodal
    coupled_variables = 'ref_all cached_all ref_boundary cached_boundary ref_from_block cached_from_block
                         ref_missed cached_missed'
    expression = 'abs(ref_all - cached_all) + abs(ref_boundary - cached_boundary)
                  + abs(ref_from_block - cached_from_block) + abs(ref_missed - cached_missed)'
    execute_on = timestep_end
  []
  [diff_block]
    type = ParsedAux
    variable = diff_block
    coupled_variables = 'ref_block cached_block'
    expression = 'abs(ref_block - cached_block)'
    block = 1
    execute_on = timestep_end
  []
  [diff_elem]
    type = ParsedAux
    variable = diff_elem
    coupled_variables = 'ref_elem cached_elem'
    expression = 'abs(ref_elem - cached_elem)'
    execute_on = timestep_end
  []
[]

[MultiApps]
  [sub]
    type = TransientMultiApp
    input_files = sub.i
    execute_on = timestep_begin
  []
  [box]
    type = TransientMultiApp
    input_files = box.i
    execute_on = timestep_begin
  []
[]

[Transfers]
  [ref_all]
    type = MultiAppGeneralFieldNearestLocationTransfer
    from_multi_app = sub
    source_variable = src
    variable = ref_all
    displaced_source_mesh = true
  []
  [cached_all]
    type = MultiAppCachedNearestLocationTransfer
    from_multi_app = sub
    source_variable = src
    variable = cached_all
    displaced_source_mesh = true
  []
  [ref_block]
    type = MultiAppGeneralFieldNearestLocationTransfer
    from_multi_app = sub
    source_variable = src
    variable = ref_block
    to_blocks = 1
    displaced_source_mesh = true
  []
  [cached_block]
    type = MultiAppCachedNearestLocationTransfer
    from_multi_app = sub
    source_variable = src
    variable = cached_block
    to_blocks = 1
    displaced_source_mesh = true
  []
  [ref_boundary]
    type = MultiAppGeneralFieldNearestLocationTransfer
    from_multi_app = sub
    source_variable = src
    variable = ref_boundary
    to_boundaries = 'left top'
    displaced_source_mesh = true
  []
  [cached_boundary]
    type = MultiAppCachedNearestLocationTransfer
    from_multi_app = sub
    source_variable = src
    variable = cached_boundary
    to_boundaries = 'left top'
    displaced_source_mesh = true
  []
  [ref_from_block]
    type = MultiAppGeneralFieldNearestLocationTransfer
    from_multi_app = sub
    source_variable = src
    variable = ref_from_block
    from_blocks = 1
    displaced_source_mesh = true
  []
  [cached_from_block]
    type = MultiAppCachedNearestLocationTransfer
    from_multi_app = sub
    source_variable = src
    variable = cached_from_block
    from_blocks = 1
    displaced_source_mesh = true
  []
  [ref_elem]
    type = MultiAppGeneralFieldNearestLocationTransfer
    from_multi_app = sub
    source_variable = src
    variable = ref_elem
    displaced_source_mesh = true
  []
  [cached_elem]
    type = MultiAppCachedNearestLocationTransfer
    from_multi_app = sub
    source_variable = src
    variable = cached_elem
    displaced_source_mesh = true
  []
  [ref_missed]
    type = MultiAppGeneralFieldNearestLocationTransfer
    from_multi_app = box
    source_variable = src
    variable = ref_missed
    from_app_must_contain_point = true
    error_on_miss = false
  []
  [cached_missed]
    type = MultiAppCachedNearestLocationTransfer
    from_multi_app = box
    source_variable = src
    variable = cached_missed
    from_app_must_contain_point = true
    error_on_miss = false
  []
[]

[Postprocessors]
  [max_diff_nodal]
    type = NodalExtremeValue
    variable = diff_nodal
  []
  [max_diff_block]
    type = NodalExtremeValue
    variable = diff_block
    block = 1
  []
  [max_diff_elem]
    type = ElementExtremeValue
    variable = diff_elem
  []
  [sum_cached_all]
    type = NodalSum
    variable = cached_all
  []
  [sum_cached_block]
    type = NodalSum
    variable = cached_block
    block = 1
  []
  [sum_cached_boundary]
    type = NodalSum
    variable = cached_boundary
  []
  [sum_cached_from_block]
    type = NodalSum
    variable = cached_from_block
  []
  [min_cached_missed]
    type = NodalExtremeValue
    variable = cached_missed
    value_type = min
  []
  [average_cached_elem]
    type = ElementAverageValue
    variable = cached_elem
  []
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Transient
  num_steps = 3
[]

[Outputs]
  csv = true
[]
