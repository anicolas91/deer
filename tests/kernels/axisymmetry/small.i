# Simple test of axisymmetry

[Problem]
  coord_type = RZ
[]

[Mesh]
  type = FileMesh
  file = 'L.exo'
[]

[NEMLMechanics]
  displacements = "disp_r disp_z"
  kinematics = small
  add_all_output = true
  type = axisymmetric
[]

[Functions]
  [./ramp]
    type = PiecewiseLinear
    x = '0 1'
    y = '0 10'
  [../]
[]

[BCs]
  [./left]
    type = PresetBC
    variable = disp_r
    boundary = left
    value = 0.0
  [../]
  [./bottom]
    type = PresetBC
    variable = disp_z
    boundary = bottom
    value = 0.0
  [../]
  [./Pressure]
    [./inside]
      boundary = inside
      function = ramp
      displacements = 'disp_r disp_z'
    [../]
  [../]
[]

[Materials]
  [./stress]
    type = ComputeNEMLStressUpdate
    database = "test.xml"
    model = "elastic_model"
    large_kinematics = false
  [../]
[]

[Preconditioning]
  [./smp]
    type = FDP
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
  nl_abs_tol = 1e-10

  dt = 1
  end_time = 1.0
[]

[Outputs]
  exodus = true
[]
