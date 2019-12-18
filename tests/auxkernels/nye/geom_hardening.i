# Simple 3D test

[GlobalParams]
  displacements = "disp_x disp_y disp_z"
[]

[Mesh]
  type = FileMesh
  file = "small.exo"
[]

[NEMLMechanics]
  kinematics = large
  add_all_output = true
[]

[NEMLCP]
  add_orientation_output = true
  add_Fe = true
  add_nye = true
[]

[UserObjects]
  [./euler_angle_file]
    type = EulerAngleFileReader
    file_name = 2gr_Euler.tex
  [../]
[]
[BCs]
  [./leftx]
    type = PresetBC
    boundary = left
    variable = disp_x
    value = 0.0
  [../]
  [./lefty]
    type = PresetBC
    boundary = back
    variable = disp_y
    value = 0.0
  [../]
  [./leftz]
    type = PresetBC
    boundary = bottom
    variable = disp_z
    value = 0.0
  [../]
  [./pull_z]
    type = FunctionPresetBC
    boundary = top
    variable = disp_z
    function = pfn
  [../]
[]

[Functions]
  [./pfn]
    type = PiecewiseLinear
    x = '0    10'
    y = '0.00 0.1'
  [../]
[]

[Materials]
  [./stress1]
    type = ComputeNEMLCPOutput
    database = "cp.xml"
    model = "geom"
    large_kinematics = true
    euler_angle_provider = euler_angle_file
    nye_tensor = 'nye_x-x nye_x-y nye_x-z nye_y-x nye_y-y nye_y-z nye_z-x nye_z-y nye_z-z'
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

  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'

  nl_max_its = 15
  nl_rel_tol = 1e-8
  nl_abs_tol = 1e-10

  start_time = 0
  dt = 0.1
  end_time = 0.4
[]

[Outputs]
  exodus = true
[]
