//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Material.h"

// Forward Declarations
class INSStressStrain : public Material

{
public:
  static InputParameters validParams();

  INSStressStrain(const InputParameters & parameters);

protected:
  virtual void computeQpProperties() override;

  /// velocity
  const VectorVariableValue & _velocity;

  /// gradient of velocity
  const VectorVariableGradient & _grad_velocity;

  //  The stresses and pressure
  MaterialProperty<RankTwoTensor> & _stress;
  MaterialProperty<RankTwoTensor> & _strain_rate;
  const VariableValue & _pressure;

  // Other
  const MaterialProperty<Real> & _mu; //converted from AD
  // const Moose::CoordinateSystemType & _coord_sys;

};
