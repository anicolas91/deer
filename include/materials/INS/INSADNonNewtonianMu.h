//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADMaterial.h"

// Forward Declarations
class INSADNonNewtonianMu : public ADMaterial

{
public:
  static InputParameters validParams();

  INSADNonNewtonianMu(const InputParameters & parameters);

protected:
  virtual void computeQpProperties() override;

  /// velocity
  const ADVectorVariableValue & _velocity;

  /// gradient of velocity
  const ADVectorVariableGradient & _grad_velocity;


  const Real _A_val;
  const Real _mexp;
  ADMaterialProperty<Real> & _mu; //converted from AD
  const Moose::CoordinateSystemType & _coord_sys;

};
