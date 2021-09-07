[Mesh]
  [./square]
    type = FileMeshGenerator
    file = 'square.e'
  []
  [./guide]
    type = RenameBlockGenerator
    input = 'square'
    old_block_id = '0'
    new_block_name = 'guide'
  []
[]
