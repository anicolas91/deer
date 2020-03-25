[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
[]

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
[]

[AuxVariables]
  [./Nneighbor_x]
    family = MONOMIAL
    order = CONSTANT
  []
  [./Nneighbor_y]
    family = MONOMIAL
    order = CONSTANT
  []
  [./Nneighbor_z]
    family = MONOMIAL
    order = CONSTANT
  []
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
  [./R_x]
    family = LAGRANGE
    order = FIRST
  []
  [./R_y]
    family = LAGRANGE
    order = FIRST
  []
  [./R_z]
    family = LAGRANGE
    order = FIRST
  []
  [./R_neigh_x]
    family = LAGRANGE
    order = FIRST
  []
  [./R_neigh_y]
    family = LAGRANGE
    order = FIRST
  []
  [./R_neigh_z]
    family = LAGRANGE
    order = FIRST
  []
[]


[AuxKernels]
  [./aux_Nn_x]
    type = MaterialRealVectorValueAux
    boundary = 'interface'
    property = n_neighbor_czm
    component = 0
    execute_on = 'TIMESTEP_END'
    variable = Nneighbor_x
  [../]
  [./aux_Nn_y]
    type = MaterialRealVectorValueAux
    boundary = 'interface'
    property = n_neighbor_czm
    component = 1
    execute_on = 'TIMESTEP_END'
    variable = Nneighbor_y
  [../]
  [./aux_Nn_z]
    type = MaterialRealVectorValueAux
    boundary = 'interface'
    property = n_neighbor_czm
    component = 2
    execute_on = 'TIMESTEP_END'
    variable = Nneighbor_z
  [../]

  [./aux_N_x]
    type = MaterialRealVectorValueAux
    boundary = 'interface'
    property = n_master_czm
    component = 0
    execute_on = 'TIMESTEP_END'
    variable = N_x
  [../]
  [./aux_N_y]
    type = MaterialRealVectorValueAux
    boundary = 'interface'
    property = n_master_czm
    component = 1
    execute_on = 'TIMESTEP_END'
    variable = N_y
  [../]
  [./aux_N_z]
    type = MaterialRealVectorValueAux
    boundary = 'interface'
    property = n_master_czm
    component = 2
    execute_on = 'TIMESTEP_END'
    variable = N_z
  [../]

  [./aux_Navg_x]
    type = MaterialRealVectorValueAux
    boundary = 'interface'
    property = n_avg_czm
    component = 0
    execute_on = 'TIMESTEP_END'
    variable = Navg_x
  [../]
  [./aux_Navg_y]
    type = MaterialRealVectorValueAux
    boundary = 'interface'
    property = n_avg_czm
    component = 1
    execute_on = 'TIMESTEP_END'
    variable = Navg_y
  [../]
  [./aux_Navg_z]
    type = MaterialRealVectorValueAux
    boundary = 'interface'
    property = n_avg_czm
    component = 2
    execute_on = 'TIMESTEP_END'
    variable = Navg_z
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
[InterfaceKernels]
  [./x]
    type = CZMInterfaceKernelLD
    component = 0
    displacements = 'disp_x disp_y disp_z'
    use_displaced_mesh = true
    variable = disp_x
    neighbor_var = disp_x
    boundary = 'interface'
    save_in = 'R_x R_neigh_x'
    save_in_var_side = 'm s'
  []
  [./y]
    type = CZMInterfaceKernelLD
    component = 1
    displacements = 'disp_x disp_y disp_z'
    use_displaced_mesh = true
    variable = disp_y
    neighbor_var = disp_y
    boundary = 'interface'
    save_in = 'R_y R_neigh_y'
    save_in_var_side = 'm s'
  []
  [./z]
    type = CZMInterfaceKernelLD
    component = 2
    displacements = 'disp_x disp_y disp_z'
    use_displaced_mesh = true
    variable = disp_z
    neighbor_var = disp_z
    boundary = 'interface'
    save_in = 'R_z R_neigh_z'
    save_in_var_side = 'm s'
  []
[]
[Materials]
  [./stress]
    type = ComputeNEMLStressUpdate
    database = "../../neml_test_material.xml"
    model = "elastic_model"
    large_kinematics = true
  [../]
  [./czm]
    type = CZMMaterialLargeDefBase
    displacements = 'disp_x disp_y disp_z'
    boundary = 'interface'
    large_kinematics = true
    K_interface = 1e4
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
  dtmin = 0.01
  dtmax = 1
  end_time = 2.5
[]

[Functions]

  [./move_x]
    type = ParsedFunction
    value = 0.1*t
  [../]
  [./move_x2]
    type = ParsedFunction
    value = -0.1*t
  [../]
[]


[BCs]
  [./x_move]
    type = FunctionDirichletBC
    boundary = 'top_nodes_right'
    variable = disp_x
    function = move_x
  [../]
  # [./x_move_2]
  #   type = FunctionDirichletBC
  #   boundary = 'bottom_nodes_right'
  #   variable = disp_x
  #   function = move_x2
  # [../]


  [./x_fix]
    type = DirichletBC
    boundary = 'left'
    variable = disp_x
    value = 0.0
  [../]
  [./y_fix]
    type = DirichletBC
    boundary = 'bottom '
    variable = disp_y
    value = 0.0
  [../]
  [./z_fix]
    type = DirichletBC
    boundary = 'back'
    variable = disp_z
    value = 0.0
  [../]
[]



[Outputs]
  exodus = true
  sync_times = '0 0.5 1 1.5 2'
[]
