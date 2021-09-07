[Mesh]
  [./square]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 2
    ny = 2
    xmin = -.63
    xmax = .63
    ymin = -.63
    ymax = .63
  []
  [./rename_boundary]
    type = RenameBoundaryGenerator
    input = square
    old_boundary = '0 1 2 3'
    new_boundary = '3 1 2 0'
  []
[]
