[Mesh]
  [./msh]
    type =FileMeshGenerator
    file = testTri10.e
  []
  [./scale]
    type = TransformGenerator
    input = msh
    transform = SCALE
    vector_value ='0.1 0.1 0.1'
  []
  [./split]
    type = BreakMeshByBlockGenerator
    input = scale
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
    new_boundary = 'bottom top left right back front'
  []
  [./lower]
     input = sides
     type = LowerDBlockFromSidesetGenerator
     new_block_name = 'LD_interface'
     new_block_id = 1000
     sidesets = 0
  [../]
[]



[NEMLMechanics]
  displacements = "disp_x disp_y disp_z"
  block = '1 2 3 4 5 6 7 8 9 10'
  kinematics = large
  add_all_output = true
[]

[Modules/TensorMechanics/CohesiveZoneMaster]
  [./czm]
    boundary = 'interface'
    displacements = 'disp_x disp_y disp_z'
  [../]
[]

[DummyInterface]
[]

[BCs]
  [./x]
    type = DirichletBC
    boundary = left
    variable = disp_x
    value = 0.0
  [../]
  [./y]
    type = DirichletBC
    boundary = bottom
    variable = disp_y
    value = 0.0
  [../]
  [./z]
    type = DirichletBC
    boundary = back
    variable = disp_z
    value = 0.0
  [../]
  [./x1]
    type = FunctionNeumannBC
    boundary = right
    function = applied_load_x
    variable = disp_x
  [../]
  [./y1]
    type = FunctionNeumannBC
    boundary = top
    function = applied_load_y
    variable = disp_y
  [../]
  [./z1]
    type = FunctionNeumannBC
    boundary = front
    function = applied_load_z
    variable = disp_z
  [../]
[]

# Constraint System
[Constraints]
  [./x1]
    type = EqualValueBoundaryConstraint
    variable = disp_x
    slave = 'right'    # boundary
    penalty = 1e6
  [../]
  [./y1]
    type = EqualValueBoundaryConstraint
    variable = disp_y
    slave = 'top'    # boundary
    penalty = 1e6
  [../]
  [./z1]
    type = EqualValueBoundaryConstraint
    variable = disp_z
    slave = 'front'    # boundary
    penalty = 1e6
  [../]
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
    y = '0 180 180'
  [../]
[]

[UserObjects]
  [./Tn_uo]
    type = Map2LDInterfaceMPRealVectorValue
    boundary = 'interface'
    mp_name = 'traction'
    ld_block_names = 'LD_interface'
    component = 0
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [./Ts1_uo]
    type = Map2LDInterfaceMPRealVectorValue
    boundary = 'interface'
    mp_name = 'traction'
    ld_block_names = 'LD_interface'
    component = 1
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [./Ts2_uo]
    type = Map2LDInterfaceMPRealVectorValue
    boundary = 'interface'
    mp_name = 'traction'
    ld_block_names = 'LD_interface'
    component = 2
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [./a_uo]
    type = Map2LDInterfaceMPReal
    boundary = 'interface'
    mp_name = 'cavity_half_radius'
    ld_block_names = 'LD_interface'
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [./b_uo]
    type = Map2LDInterfaceMPReal
    boundary = 'interface'
    mp_name = 'cavity_half_spacing'
    ld_block_names = 'LD_interface'
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [./D_uo]
    type = Map2LDInterfaceMPReal
    boundary = 'interface'
    mp_name = 'interface_damage'
    ld_block_names = 'LD_interface'
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [./avg_eq_strain_rate_interface]
    type = Map2LDInterfaceMPReal
    boundary = 'interface'
    mp_name = 'eq_inelastic_strain_rate'
    ld_block_names = 'LD_interface'
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [./avg_stress_vm_interface]
    type = Map2LDInterfaceMPReal
    boundary = 'interface'
    mp_name = 'stress_VM'
    ld_block_names = 'LD_interface'
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [./avg_stress_h_interface]
    type = Map2LDInterfaceMPReal
    boundary = 'interface'
    mp_name = 'stress_H'
    ld_block_names = 'LD_interface'
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [./acc_strain_interface_uo]
    type = Map2LDInterfaceMPReal
    boundary = 'interface'
    mp_name = 'acc_inelastic_strain'
    ld_block_names = 'LD_interface'
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [./strain_rate_interface_uo]
    type = Map2LDInterfaceMPReal
    boundary = 'interface'
    mp_name = 'eq_inelastic_strain_rate_interface'
    ld_block_names = 'LD_interface'
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [./nucleation_achieved_uo]
    type = Map2LDInterfaceMPReal
    boundary = 'interface'
    mp_name = 'nucleation_threshold_achieved'
    ld_block_names = 'LD_interface'
    execute_on = 'INITIAL TIMESTEP_END'
  []
[]


[AuxVariables]
  [./nucleation_threshold_achieved]
    family = MONOMIAL
    order = CONSTANT
  []
  [./T_N]
    family = MONOMIAL
    order = CONSTANT
  []
  [./T_S1]
    family = MONOMIAL
    order = CONSTANT
  []
  [./T_S2]
    family = MONOMIAL
    order = CONSTANT
  []
  [./a]
    family = MONOMIAL
    order = CONSTANT
  []
  [./b]
    family = MONOMIAL
    order = CONSTANT
  []
  [./D]
    family = MONOMIAL
    order = CONSTANT
  []
  [./stress_VM_interface]
    order = CONSTANT
    family = MONOMIAL
  []
  [./stress_VM]
    order = CONSTANT
    family = MONOMIAL
  []
  [./stress_H]
    order = CONSTANT
    family = MONOMIAL
  []
  [./stress_H_interface]
    order = CONSTANT
    family = MONOMIAL
  []
  [./eq_inelastic_strain_rate_interface]
    order = CONSTANT
    family = MONOMIAL
  []
  [./acc_inelastic_strain_interface]
    order = CONSTANT
    family = MONOMIAL
  []
[]

[AuxKernels]
  [nucleation_aux]
    type = Boundary2LDAux
    map2LDelem_uo_name = nucleation_achieved_uo
    variable = nucleation_threshold_achieved
    block = 'LD_interface'
    execute_on = 'TIMESTEP_END'
  []

  [Tn_aux]
    type = Boundary2LDAux
    map2LDelem_uo_name = Tn_uo
    variable = T_N
    block = 'LD_interface'
    execute_on = 'TIMESTEP_END'
  []
  [Ts1_aux]
    type = Boundary2LDAux
    map2LDelem_uo_name = Ts1_uo
    variable = T_S1
    block = 'LD_interface'
    execute_on = 'TIMESTEP_END'
  []
  [Ts2_aux]
    type = Boundary2LDAux
    map2LDelem_uo_name = Ts2_uo
    variable = T_S2
    block = 'LD_interface'
    execute_on = 'TIMESTEP_END'
  []
  [a_aux]
    type = Boundary2LDAux
    map2LDelem_uo_name = a_uo
    variable = a
    block = 'LD_interface'
    execute_on = 'TIMESTEP_END'
  []
  [b_aux]
    type = Boundary2LDAux
    map2LDelem_uo_name = b_uo
    variable = b
    block = 'LD_interface'
    execute_on = 'TIMESTEP_END'
  []
  [D_aux]
    type = Boundary2LDAux
    map2LDelem_uo_name = D_uo
    variable = D
    block = 'LD_interface'
    execute_on = 'TIMESTEP_END'
  []
  [stress_VM_interface_aux]
    type = Boundary2LDAux
    map2LDelem_uo_name = avg_stress_vm_interface
    variable = stress_VM_interface
    block = 'LD_interface'
    execute_on = 'TIMESTEP_END'
  []
  [stress_VM_aux]
    type = RankTwoScalarAux
    variable = stress_VM
    rank_two_tensor = 'stress'
    scalar_type = 'VonMisesStress'
    execute_on = 'TIMESTEP_END'
    block = '1 2 3 4 5 6 7 8 9 10'
  []
  [stress_H_aux]
    type = RankTwoScalarAux
    variable = stress_H
    rank_two_tensor = 'stress'
    scalar_type = 'Hydrostatic'
    execute_on = 'TIMESTEP_END'
    block = '1 2 3 4 5 6 7 8 9 10'
  []
  [stress_H_interface_aux]
    type = Boundary2LDAux
    map2LDelem_uo_name = avg_stress_h_interface
    variable = stress_H_interface
    block = 'LD_interface'
    execute_on = 'TIMESTEP_END'
  []
  [strain_rate_aux]
    type = Boundary2LDAux
    map2LDelem_uo_name = strain_rate_interface_uo
    variable = eq_inelastic_strain_rate_interface
    block = 'LD_interface'
    execute_on = 'TIMESTEP_END'
  []
  [acc_strain_aux]
    type = Boundary2LDAux
    map2LDelem_uo_name = acc_strain_interface_uo
    variable = acc_inelastic_strain_interface
    block = 'LD_interface'
    execute_on = 'TIMESTEP_END'
  []
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
    type = GBCavitation
    displacements = 'disp_x disp_y disp_z'
    boundary = 'interface'
    E = 150e3
    G = 52e3
    shear_viscosity = 1e6
    interface_thickness = 0.001
    eq_inelastic_strain_rate_uo_name = avg_eq_strain_rate_interface
    avg_stressVM_uo_name = avg_stress_vm_interface
    avg_stressH_uo_name = avg_stress_h_interface
    snes_use_FD = false
    linesearch_type = 0
    nucleation_is_active = true
    growth_is_active = true
    VL_VH = 0
    use_vdot_triax = 0
    snes_rel_tol = 1e-8
    snes_abs_tol = 1e-8
    penetration_penalty = 10
    stiffness_reduction_factor =1e5
    residual_life_scaling_factor = 1
    scale_equations = 0
    max_damage = 0.5
    use_lm = 1

    min_allowed_residual_life = 10
  [../]

  [eq_strain_rate_interface]
    type = ScalarFromTensorRateInterface
    rank_two_tensor = 'inelastic_strain'
    new_scalar_mp_name = 'eq_inelastic_strain_rate'
    scalar_type = 'EffectiveStrain'
    boundary = 'interface'
  []
  [stress_vm_interface]
    type = ScalarFromTensorInterface
    rank_two_tensor = 'stress'
    new_scalar_mp_name = 'stress_VM'
    scalar_type = 'VonMisesStress'
    boundary = 'interface'
  []
  [stress_H_interface]
    type = ScalarFromTensorInterface
    rank_two_tensor = 'stress'
    new_scalar_mp_name = 'stress_H'
    scalar_type = 'Hydrostatic'
    boundary = 'interface'
  []
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
  petsc_options_value = 'lu '
  [./TimeStepper]
   type =IterationAdaptiveDT
    dt =  0.1
  []
  l_max_its = 2
  l_tol = 1e-14
  nl_max_its = 10
  nl_rel_tol = 1e-6
  nl_abs_tol = 1e-6
  # dtmin = 0.001
  dtmax = 10000
  end_time = 2e5
[]

[Outputs]
  exodus = true
  sync_times = '0 1 5 10 100 200 500 1000 2000'
[]
