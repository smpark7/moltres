[Mesh]
  [./fuel_pin]
    type = FileMeshGenerator
    file = fuel_pin.e
  []
  [./rename_block]
    type = RenameBlockGenerator
    input = fuel_pin
    old_block_id = '1 2'
    new_block_name = 'fuel mod'
  []
[]
