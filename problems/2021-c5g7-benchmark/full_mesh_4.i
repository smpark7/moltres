[Mesh]
  # Import fuel pins, guide, chamber, and moderator
  [./uo2]
    type = FileMeshGenerator
    file = uo2_pin.e
  []
  [./mox1]
    type = FileMeshGenerator
    file = mox1_pin.e
  []
  [./mox2]
    type = FileMeshGenerator
    file = mox2_pin.e
  []
  [./mox3]
    type = FileMeshGenerator
    file = mox3_pin.e
  []
  [./guide]
    type = FileMeshGenerator
    file = guide.e
  []
  [./chamber]
    type = FileMeshGenerator
    file = chamber.e
  []
  [./moderator]
    type = FileMeshGenerator
    file = mod.e
  []

  # Build rows of pins in each assembly
  [./row1]
    type = PatternedMeshGenerator
    inputs = 'uo2'
    pattern = '0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0'
  []
  [./row2]
    type = PatternedMeshGenerator
    inputs = 'uo2 guide'
    pattern = '0 0 0 0 0 1 0 0 1 0 0 1 0 0 0 0 0'
  []
  [./row3]
    type = PatternedMeshGenerator
    inputs = 'uo2 guide'
    pattern = '0 0 0 1 0 0 0 0 0 0 0 0 0 1 0 0 0'
  []
  [./row4]
    type = PatternedMeshGenerator
    inputs = 'uo2 guide'
    pattern = '0 0 1 0 0 1 0 0 1 0 0 1 0 0 1 0 0'
  []
  [./row5]
    type = PatternedMeshGenerator
    inputs = 'uo2 guide chamber'
    pattern = '0 0 1 0 0 1 0 0 2 0 0 1 0 0 1 0 0'
  []

  [./row6]
    type = PatternedMeshGenerator
    inputs = 'mox3'
    pattern = '0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0'
  []
  [./row7]
    type = PatternedMeshGenerator
    inputs = 'mox3 mox2'
    pattern = '0 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 0'
  []
  [./row8]
    type = PatternedMeshGenerator
    inputs = 'mox3 mox2 guide'
    pattern = '0 1 1 1 1 2 1 1 2 1 1 2 1 1 1 1 0'
  []
  [./row9]
    type = PatternedMeshGenerator
    inputs = 'mox3 mox2 mox1 guide'
    pattern = '0 1 1 3 1 2 2 2 2 2 2 2 1 3 1 1 0'
  []
  [./row10]
    type = PatternedMeshGenerator
    inputs = 'mox3 mox2 mox1'
    pattern = '0 1 1 1 2 2 2 2 2 2 2 2 2 1 1 1 0'
  []
  [./row11]
    type = PatternedMeshGenerator
    inputs = 'mox3 mox2 mox1 guide'
    pattern = '0 1 3 2 2 3 2 2 3 2 2 3 2 2 3 1 0'
  []
  [./row12]
    type = PatternedMeshGenerator
    inputs = 'mox3 mox2 mox1'
    pattern = '0 1 1 2 2 2 2 2 2 2 2 2 2 2 1 1 0'
  []
  [./row13]
    type = PatternedMeshGenerator
    inputs = 'mox3 mox2 mox1 guide chamber'
    pattern = '0 1 3 2 2 3 2 2 4 2 2 3 2 2 3 1 0'
  []

  [./row14]
    type = PatternedMeshGenerator
    inputs = 'moderator'
    pattern = '0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0'
  []

  # Build assemblies
  [./uo2_assembly]
    type = PatternedMeshGenerator
    inputs = 'row1 row2 row3 row4 row5'
    pattern = '0;
               0;
               1;
               2;
               0;
               3;
               0;
               0;
               4;
               0;
               0;
               3;
               0;
               2;
               1;
               0;
               0'
  []
  [./mox_assembly]
    type = PatternedMeshGenerator
    inputs = 'row6 row7 row8 row9 row10 row11 row12 row13'
    pattern = '0;
               1;
               2;
               3;
               4;
               5;
               6;
               6;
               7;
               6;
               6;
               5;
               4;
               3;
               2;
               1;
               0'
  []
  [./mod_assembly]
    type = PatternedMeshGenerator
    inputs = 'row14'
    pattern = '0;
               0;
               0;
               0;
               0;
               0;
               0;
               0;
               0;
               0;
               0;
               0;
               0;
               0;
               0;
               0;
               0'
  []

  # Build rows of assemblies
  [./top_row]
    type = PatternedMeshGenerator
    inputs = 'uo2_assembly mox_assembly mod_assembly'
    pattern = '0 1 2'
  []
  [./mid_row]
    type = PatternedMeshGenerator
    inputs = 'uo2_assembly mox_assembly mod_assembly'
    pattern = '1 0 2'
  []
  [./bottom_row]
    type = PatternedMeshGenerator
    inputs = 'mod_assembly'
    pattern = '0 0 0'
  []

  # Build full mesh
  [./full_mesh]
    type = PatternedMeshGenerator
    inputs = 'top_row mid_row bottom_row'
    pattern = '0;
               1;
               2'
  []

# The code below doesn't work due to a bug
#  [./full_mesh]
#    type = PatternedMeshGenerator
#    #         0   1    2    3    4     5       6
#    inputs = 'uo2 mox1 mox2 mox3 rename_guide rename_chamber rename_moderator'
#    pattern = '0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 ;
#               0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 3 2 2 2 2 2 2 2 2 2 2 2 2 2 2 2 3 ;
#               0 0 0 0 0 4 0 0 4 0 0 4 0 0 0 0 0 3 2 2 2 2 4 2 2 4 2 2 4 2 2 2 2 3 ;
#               0 0 0 4 0 0 0 0 0 0 0 0 0 4 0 0 0 3 2 2 4 2 1 1 1 1 1 1 1 2 4 2 2 3 ;
#               0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 3 2 2 2 1 1 1 1 1 1 1 1 1 2 2 2 3 ;
#               0 0 4 0 0 4 0 0 4 0 0 4 0 0 4 0 0 3 2 4 1 1 4 1 1 4 1 1 4 1 1 4 2 3 ;
#               0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 3 2 2 1 1 1 1 1 1 1 1 1 1 1 2 2 3 ;
#               0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 3 2 2 1 1 1 1 1 1 1 1 1 1 1 2 2 3 ;
#               0 0 4 0 0 4 0 0 5 0 0 4 0 0 4 0 0 3 2 4 1 1 4 1 1 5 1 1 4 1 1 4 2 3 ;
#               0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 3 2 2 1 1 1 1 1 1 1 1 1 1 1 2 2 3 ;
#               0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 3 2 2 1 1 1 1 1 1 1 1 1 1 1 2 2 3 ;
#               0 0 4 0 0 4 0 0 4 0 0 4 0 0 4 0 0 3 2 4 1 1 4 1 1 4 1 1 4 1 1 4 2 3 ;
#               0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 3 2 2 2 1 1 1 1 1 1 1 1 1 2 2 2 3 ;
#               0 0 0 4 0 0 0 0 0 0 0 0 0 4 0 0 0 3 2 2 4 2 1 1 1 1 1 1 1 2 4 2 2 3 ;
#               0 0 0 0 0 4 0 0 4 0 0 4 0 0 0 0 0 3 2 2 2 2 4 2 2 4 2 2 4 2 2 2 2 3 ;
#               0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 3 2 2 2 2 2 2 2 2 2 2 2 2 2 2 2 3 ;
#               0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 ;
#               3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 ;
#               3 2 2 2 2 2 2 2 2 2 2 2 2 2 2 2 3 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 ;
#               3 2 2 2 2 4 2 2 4 2 2 4 2 2 2 2 3 0 0 0 0 0 4 0 0 4 0 0 4 0 0 0 0 0 ;
#               3 2 2 4 2 1 1 1 1 1 1 1 2 4 2 2 3 0 0 0 4 0 0 0 0 0 0 0 0 0 4 0 0 0 ;
#               3 2 2 2 1 1 1 1 1 1 1 1 1 2 2 2 3 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 ;
#               3 2 4 1 1 4 1 1 4 1 1 4 1 1 4 2 3 0 0 4 0 0 4 0 0 4 0 0 4 0 0 4 0 0 ;
#               3 2 2 1 1 1 1 1 1 1 1 1 1 1 2 2 3 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 ;
#               3 2 2 1 1 1 1 1 1 1 1 1 1 1 2 2 3 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 ;
#               3 2 4 1 1 4 1 1 5 1 1 4 1 1 4 2 3 0 0 4 0 0 4 0 0 5 0 0 4 0 0 4 0 0 ;
#               3 2 2 1 1 1 1 1 1 1 1 1 1 1 2 2 3 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 ;
#               3 2 2 1 1 1 1 1 1 1 1 1 1 1 2 2 3 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 ;
#               3 2 4 1 1 4 1 1 4 1 1 4 1 1 4 2 3 0 0 4 0 0 4 0 0 4 0 0 4 0 0 4 0 0 ;
#               3 2 2 2 1 1 1 1 1 1 1 1 1 2 2 2 3 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 ;
#               3 2 2 4 2 1 1 1 1 1 1 1 2 4 2 2 3 0 0 0 4 0 0 0 0 0 0 0 0 0 4 0 0 0 ;
#               3 2 2 2 2 4 2 2 4 2 2 4 2 2 2 2 3 0 0 0 0 0 4 0 0 4 0 0 4 0 0 0 0 0 ;
#               3 2 2 2 2 2 2 2 2 2 2 2 2 2 2 2 3 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 ;
#               3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0'
#    bottom_boundary = 1
#    right_boundary = 2
#    top_boundary = 3
#    left_boundary = 4
#  []
[]
