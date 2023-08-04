[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
  incremental = true
  eigenstrain_name = eigenstrain
  temperature = temperature
  family=LAGRANGE
  order=FIRST
[]

[Mesh]
  type = GeneratedMesh
  dim = 3
  nx = 10
  ny = 10
  nz = 10
  xmax = 0.06
  ymax = 0.06
  zmax = 0.06 
  elem_type= HEX8
[]

[Functions]
  [source]
    type = ParsedFunction
    expression = 600
  [../]
[]


[Variables]
  [./temperature]
    initial_condition = 500
  [../]
  [./disp_x]
  [../]
  [./disp_y]
  [../]
  [./disp_z]
  [../]
[]

[Kernels]
  [heat_conduction]
    type = HeatConduction
    variable = temperature
  []
  [T_source]
    type = HeatSource
    variable =  temperature
    function = source
  []
  [heat_conduction_time_derivative]
    type = HeatConductionTimeDerivative
    variable = temperature
  []
[]

[BCs]
  [confinex]
    type = DirichletBC
    variable = disp_x
    value = 0
    boundary = 0
  []
  [confiney]
    type = DirichletBC
    variable = disp_y
    value = 0
    boundary = 0
  []
  [confinez]
    type = DirichletBC
    variable = disp_z
    value = 0
    boundary = 0
  []
  [./templ]
    type = DirichletBC
    variable = temperature
    value =600
    boundary = 'back'
  []
[]


[Materials]
   [fuel_elasticity_tensor]
      type = ComputeIsotropicElasticityTensor
      youngs_modulus = 7.77e10
      poissons_ratio = 0.323
    [../]
    [./thermal_strain1]
      type = ComputeThermalExpansionEigenstrain
      thermal_expansion_coeff = 1.243e-5
      stress_free_temperature = 230
      temperature = temperature
    [../]
    [fuel_stress]
      type = ComputeFiniteStrainElasticStress
    []
    [density]
      type = GenericConstantMaterial
      prop_names = 'density specific_heat  thermal_conductivity'
      prop_values = '3900 595 400'
    []
[]

[Modules]
  [TensorMechanics]
    [Master]
      [fule]
        strain = FINITE
        eigenstrain_names = eigenstrain
        add_variables = true
        generate_output = 'stress_xx stress_yy stress_zz stress_xy stress_xz stress_yz
                          strain_xx strain_yy strain_zz strain_xy strain_xz strain_yz'
      []
    []
  []
[]

[Preconditioning]
  active="smp"
   [./smp]
   type = SMP
   full = true
   [../]
[]

[Problem]
  type = CSMPProblem
[]

[Executioner]
  type = Transient
  scheme = bdf2
  end_time=2000
  dt=500
  auto_preconditioning=false
  solve_type =PJFNK
  petsc_options_iname = '-ksp_type -pc_type -pc_hypre_type  '#-ksp_view_pmat
  petsc_options_value = 'gmres hypre boomeramg  '#ascii:P25.m:ascii_matlab 
  l_max_its = 40
  l_tol = 8e-3
  nl_max_its = 10
  nl_rel_tol = 1e-5
  nl_abs_tol = 1e-7
[]
[Outputs]
  interval = 1
  csv = true
  exodus = true
  [./console]
    type = Console
    output_linear = true
    output_nonlinear = true
    output_file=true
    time_data=true
  [../]
[]

[Postprocessors]
 [single_nodal_temperature]
   type = PointValue
   point = '0.01 0.01 0.01'
   variable = "temperature"
 []
 [single_nodal_x]
   type = PointValue
   point = '0.01 0.01 0.01'
   variable = "disp_x"
 []
[]