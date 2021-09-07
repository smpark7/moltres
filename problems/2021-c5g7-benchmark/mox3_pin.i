[Mesh]
  [./fuel_pin]
    type = FileMeshGenerator
    file = fuel_pin.e
  []
  [./rename_block]
    type = RenameBlockGenerator
    input = fuel_pin
    old_block_id = '1 2'
    new_block_name = 'mox3 mox3_mod'
  []
  [./rename_nodeset]
    type = RenameBoundaryGenerator
    input = rename_block
    old_boundary = '1 2 3 4'
    new_boundary = 'left bottom right top'
  []
[]
