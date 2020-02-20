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
  kinematics = large
  add_all_output = true
[]

[Modules/TensorMechanics/CohesiveZoneMaster]
  [./czm]
    boundary = 'interface'
    displacements = 'disp_x disp_y disp_z'
  [../]
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
  [./svm_avg_uo]
    type = InterfaceQpMaterialPropertyRankTwoScalarUO
    boundary = 'interface'
    scalar_type = 'VonMisesStress'
    property = 'stress'
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [./sh_avg_uo]
    type = InterfaceQpMaterialPropertyRankTwoScalarUO
    boundary = 'interface'
    scalar_type = 'Hydrostatic'
    property = 'stress'
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [./eq_inelastic_strainrate_avg_uo]
    type = InterfaceQpMaterialPropertyRankTwoScalarUO
    boundary = 'interface'
    scalar_type = 'EffectiveStrain'
    property = 'inelastic_strain'
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [./euler_angle_file]
    type = EulerAngleFileReader
    file_name = random_ori.txt
  [../]
[]


[AuxVariables]
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
[]

[AuxKernels]
  [Tn_aux]
    type = MaterialRealVectorValueAux
    property = 'traction'
    component = 0
    variable = T_N
    boundary = 'interface'
    execute_on = 'TIMESTEP_END'
  []
  [Ts1_aux]
    type = MaterialRealVectorValueAux
    property = 'traction'
    component = 0
    variable = T_S1
    boundary = 'interface'
    execute_on = 'TIMESTEP_END'
  []
  [Ts2_aux]
    type = MaterialRealVectorValueAux
    property = 'traction'
    component = 0
    variable = T_S2
    boundary = 'interface'
    execute_on = 'TIMESTEP_END'
  []
  [a_aux]
    type = MaterialRealAux
    variable = a
    property = 'cavity_half_radius'
    boundary = 'interface'
    execute_on = 'TIMESTEP_END'
  []
  [b_aux]
    type = MaterialRealAux
    property = 'cavity_half_spacing'
    variable = b
    boundary = 'interface'
    execute_on = 'TIMESTEP_END'
  []
  [D_aux]
    type = MaterialRealAux
    variable = D
    property = 'interface_damage'
    boundary = 'interface'
    execute_on = 'TIMESTEP_END'
  []
[]

[Materials]
  [./stress]
    type = ComputeNEMLCPOutput
    database = "Gr91_grains_with_diffusion.xml"
    model = "grain_1"
    large_kinematics = true
    euler_angle_provider = euler_angle_file
  [../]

  [./ShamNeedleman]
     type = GBCavitation
     boundary = 'interface'
     use_old_avg_prop = true
     FN_NI = 20000
     Nmax_NI = 1000
     a_0 = 5e-05
     b_0 = 0.06
     sigma_0 = 200
     D_gb = 1e-15
     beta_exponent = 2
     E_penalty = 10
     # S_thr = 1e-6
     eta_sliding = 1e+06
     D_thr = 0.95

     # interface_avg_mises_stress = sVM_avg
     interface_avg_mises_stress_uo = svm_avg_uo
     # interface_avg_hydrostatic_stress = sH_avg
     interface_avg_hydrostatic_stress_uo = sh_avg_uo
     # interface_avg_eq_strain_rate = eceq_avg
     interface_avg_eq_strain_rate_uo = eq_inelastic_strainrate_avg_uo

     linesearch_type = 1
     snes_abs_tol = 1e-6
     snes_rel_tol = 1e-6
     snes_step_tol = 1e-6
     snes_use_FD = false

     max_allowed_opening_traction = 1400
     max_allowed_damage_rate = 0.0001
     min_allowed_residual_life = 50
     traction_mean_decay_time_factor = 0.5

     vdot_type = 1
     triaxial_vdot_active = 1
     vdot_max_type = 1
     vdot_smooth_factor = 35
     force_substep = false
     use_substep = true
     use_LM = false
     cavity_nucleation_on = true
     cavity_growth_on = true
     min_allowed_residual_stiffness = 1.4
     theta_time_integration = 0
     give_up_qp = false
     triaxial_cavity_growth_on = true
     displacements = 'disp_x disp_y disp_z'
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
  petsc_options = '-gbcavitation_snes_test_display'
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
