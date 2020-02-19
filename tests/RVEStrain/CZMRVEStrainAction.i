[Mesh]
  [./msh]
    type =FileMeshGenerator
    file = testTri10.e
  []
  [./split]
    type = BreakMeshByBlockGenerator
    input = msh
  []
  [./sides]
    input = split
    type = SideSetsFromNormalsGenerator
    normals = '0 -1  0
           0  1  0
           -1 0  0
           1  0  0
           0  0 -1
           0  0  1'
    fixed_normal = true
    new_boundary = 'y0 y1 x0 x1 z0 z1'
  []
[]


[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
[]

[Variables]
  [./disp_x]
    family = LAGRANGE
    order = SECOND
    # scaling = 1.0e4 #Scales residual to improve convergence
  []
  [./disp_y]
    family = LAGRANGE
    order = SECOND
    # scaling = 1.0e4 #Scales residual to improve convergence
  []
  [./disp_z]
    family = LAGRANGE
    order = SECOND
    # scaling = 1.0e4 #Scales residual to improve convergence
  []
[]


[Functions]
  [./applied_load_x]
    type = PiecewiseLinear
    x = '0 0.1 1e7'
    y = '0 0 0'
  [../]
  [./applied_load_y]
    type = PiecewiseLinear
    x = '0 0.1 1e7'
    y = '0 0 0'
  [../]
  [./applied_load_z]
    type = PiecewiseLinear
    x = '0 0.1 1e7'
    y = '0 200 200'
  [../]
[]


[BCs]
  [./x0]
    type = DirichletBC
    variable = disp_x
    boundary = x0
    value = 0.0
  [../]
  [./y0]
    type = DirichletBC
    variable = disp_y
    boundary = y0
    value = 0.0
  [../]
  [./z0]
    type = DirichletBC
    variable = disp_z
    boundary = z0
    value = 0.0
  [../]
  [./x1]
    type = FunctionNeumannBC
    boundary = x1
    function = applied_load_x
    variable = disp_x
  [../]
  [./y1]
    type = FunctionNeumannBC
    boundary = y1
    function = applied_load_y
    variable = disp_y
  [../]
  [./z1]
    type = FunctionNeumannBC
    boundary = z1
    function = applied_load_z
    variable = disp_z
  [../]
[]

# Constraint System
[Constraints]
  [./x1]
    type = EqualValueBoundaryConstraint
    variable = disp_x
    slave = 'x1'    # boundary
    penalty = 1e6
  [../]
  [./y1]
    type = EqualValueBoundaryConstraint
    variable = disp_y
    slave = 'y1'    # boundary
    penalty = 1e6
  [../]
  [./z1]
    type = EqualValueBoundaryConstraint
    variable = disp_z
    slave = 'z1'    # boundary
    penalty = 1e6
  [../]
[]


[NEMLMechanics]
  kinematics = small
  add_all_output = true
[]

[Modules/TensorMechanics/CohesiveZoneMaster]
  [./czm]
    boundary = 'interface'
    displacements = 'disp_x disp_y disp_z'
  [../]
[]

[./CZMRVEStrain]
  displacements = 'disp_x disp_y disp_z'
  boundary = 'interface'
  grains_block_names = '1 2 3 4 5 6 7 8 9 10'
[]

[Materials]
  [./stress1]
    type = ComputeNEMLStressUpdate
    database = "100_random_grains_with_diffusion.xml"
    model = "grain_1"
    block = 1
  [../]
  [./stress2]
    type = ComputeNEMLStressUpdate
    database = "100_random_grains_with_diffusion.xml"
    model = "grain_2"
    block = 2
  [../]
  [./stress3]
    type = ComputeNEMLStressUpdate
    database = "100_random_grains_with_diffusion.xml"
    model = "grain_3"
    block = 3
  [../]
  [./stress4]
    type = ComputeNEMLStressUpdate
    database = "100_random_grains_with_diffusion.xml"
    model = "grain_4"
    block = 4
  [../]
  [./stress5]
    type = ComputeNEMLStressUpdate
    database = "100_random_grains_with_diffusion.xml"
    model = "grain_5"
    block = 5
  [../]
  [./stress6]
    type = ComputeNEMLStressUpdate
    database = "100_random_grains_with_diffusion.xml"
    model = "grain_6"
    block = 6
  [../]
  [./stress7]
    type = ComputeNEMLStressUpdate
    database = "100_random_grains_with_diffusion.xml"
    model = "grain_7"
    block = 7
  [../]
  [./stress8]
    type = ComputeNEMLStressUpdate
    database = "100_random_grains_with_diffusion.xml"
    model = "grain_8"
    block = 8
  [../]
  [./stress9]
    type = ComputeNEMLStressUpdate
    database = "100_random_grains_with_diffusion.xml"
    model = "grain_9"
    block = 9
  [../]
  [./stress10]
    type = ComputeNEMLStressUpdate
    database = "100_random_grains_with_diffusion.xml"
    model = "grain_10"
    block = 10
  [../]

  [./czm]
    type = ViscousSlidingCZM
    displacements = 'disp_x disp_y disp_z'
    boundary = 'interface'
    E = 150e3
    G = 57.69e3
    shear_viscosity = 1e+06
    interface_thickness = 1
  [../]
[]



 [Preconditioning]
   [./SMP]
     type = SMP
     full = true
   [../]
 []
[Executioner]
  # Preconditisoned JFNK (default)
  type = Transient
  petsc_options_iname = '-pc_type -pc_factor_mat_solver_package'
  petsc_options_value = 'lu superlu_dist'
  # petsc_options = '-snes_converged_reason -ksp_converged_reason'
  solve_type = NEWTON
  nl_abs_tol = 1e-5
  nl_rel_tol = 1e-6
  nl_max_its = 7
  l_tol = 1e-17
  l_max_its = 3
  start_time = 0.0
  dtmin = 1e-3
  dtmax = 5e3
  end_time = 0.1

  [./TimeStepper]
  type = IterationAdaptiveDT
  dt = 0.01

  growth_factor = 2
  cutback_factor = 0.5
  optimal_iterations = 7
  iteration_window = 2
  [../]

  line_search = none
[]
[Outputs]
  sync_times = 0.1
  exodus =true
[]
