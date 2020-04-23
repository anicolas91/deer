[Mesh]
  [./msh]
  type = GeneratedMeshGenerator
  dim = 3
  nx = 1
  ny = 1
  nz = 2
  xmin = -1
  xmax = 0
  zmin = -1
  zmax = 1
  []
  [./new_block]
    type = SubdomainBoundingBoxGenerator
    input = msh
    block_id = 1
    bottom_left = '-1 0 0'
    top_right = '0 1 1'
  []
  [./split]
    type = BreakMeshByBlockGenerator
    input = new_block
  []
  [./top_nodes_left]
    input = split
    type = ExtraNodesetGenerator
    nodes = '8 11 12 15'
    new_boundary = top_nodes_left
  []
  [./top_nodes_right]
    input = top_nodes_left
    type = ExtraNodesetGenerator
    nodes = '9 10 13 14'
    new_boundary = top_nodes_right
  []
  [./bottom_nodes]
    input = top_nodes_right
    type = ExtraNodesetGenerator
    nodes = '0 1 2 3 4 5 6 7'
    new_boundary = bottom_nodes
  []
[]

[AuxVariables]
  [./N_x]
    family = MONOMIAL
    order = CONSTANT
  []
  [./N_y]
    family = MONOMIAL
    order = CONSTANT
  []
  [./N_z]
    family = MONOMIAL
    order = CONSTANT
  []
  [./Navg_x]
    family = MONOMIAL
    order = CONSTANT
  []
  [./Navg_y]
    family = MONOMIAL
    order = CONSTANT
  []
  [./Navg_z]
    family = MONOMIAL
    order = CONSTANT
  []
  [./Tavg_x]
    family = MONOMIAL
    order = CONSTANT
  []
  [./Tavg_y]
    family = MONOMIAL
    order = CONSTANT
  []
  [./Tavg_z]
    family = MONOMIAL
    order = CONSTANT
  []
[]


[AuxKernels]

  [./aux_Navg_x]
    type = MaterialRealVectorValueAux
    boundary = 'interface'
    property = n_avg
    component = 0
    execute_on = 'TIMESTEP_END'
    variable = Navg_x
  [../]
  [./aux_Navg_y]
    type = MaterialRealVectorValueAux
    boundary = 'interface'
    property = n_avg
    component = 1
    execute_on = 'TIMESTEP_END'
    variable = Navg_y
  [../]
  [./aux_Navg_z]
    type = MaterialRealVectorValueAux
    boundary = 'interface'
    property = n_avg
    component = 2
    execute_on = 'TIMESTEP_END'
    variable = Navg_z
  [../]
  [./aux_Tavg_x]
    type = MaterialRealVectorValueAux
    boundary = 'interface'
    property = traction_global
    component = 0
    execute_on = 'TIMESTEP_END'
    variable = Tavg_x
  [../]
  [./aux_Tavg_y]
    type = MaterialRealVectorValueAux
    boundary = 'interface'
    property = traction_global
    component = 1
    execute_on = 'TIMESTEP_END'
    variable = Tavg_y
  [../]
  [./aux_Tavg_z]
    type = MaterialRealVectorValueAux
    boundary = 'interface'
    property = traction_global
    component = 2
    execute_on = 'TIMESTEP_END'
    variable = Tavg_z
  [../]
[]


[NEMLMechanics]
  displacements = 'disp_x disp_y disp_z'
  kinematics = large
  add_all_output = true
  add_displacements = true
[]

# [Modules/TensorMechanics/CohesiveZoneMaster]
#   [./czm]
#     boundary = 'interface'
#     displacements = 'disp_x disp_y disp_z'
#   [../]
# []

[Materials]
  [./stress]
    type = ComputeNEMLStressUpdate
    database = "../../neml_test_material.xml"
    model = "elastic_model"
    large_kinematics = true
  [../]
  [./czm]
    type = CZMMaterialLD
    displacements = 'disp_x disp_y disp_z'
    boundary = 'interface'
    large_kinematics = true
    E = 1e2
    G = 1e2
    # interface_thickness = 1
  [../]
[]

[Preconditioning]
  [./smp]
    type = SMP
    full = true
  [../]
[]

[Executioner]
  type = Transient

  solve_type = 'newton'
  line_search = none

  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
  l_max_its = 2
  l_tol = 1e-14
  nl_max_its = 15
  nl_rel_tol = 1e-8
  nl_abs_tol = 1e-6
  dtmin = 1
  dtmax = 1
  end_time = 2.5
[]

[Functions]

  [./move_x]
    type = ParsedFunction
    value = 0.1*t
  [../]

[]


[BCs]
  [./x_move]
    type = FunctionDirichletBC
    boundary = 'top_nodes_right'
    variable = disp_x
    function = move_x
  [../]


  [./x_fix]
    type = DirichletBC
    boundary = 'bottom_nodes top_nodes_left'
    variable = disp_x
    value = 0.0
  [../]
  [./y_fix]
    type = DirichletBC
    boundary = 'bottom_nodes top_nodes_left top_nodes_right '
    variable = disp_y
    value = 0.0
  [../]
  [./z_fix]
    type = DirichletBC
    boundary = 'bottom_nodes top_nodes_left top_nodes_right'
    variable = disp_z
    value = 0.0
  [../]
[]



[Outputs]
  exodus = true
  sync_times = '0 0.5 1 1.5 2'
[]
