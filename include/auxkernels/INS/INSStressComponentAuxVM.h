//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "AuxKernel.h"

// Forward Declarations

/**
 * Computes h_min / |u|
 */
class INSStressComponentAuxVM : public AuxKernel
{
public:
  static InputParameters validParams();

  INSStressComponentAuxVM(const InputParameters & parameters);

  virtual ~INSStressComponentAuxVM() {}

protected:
  virtual Real computeValue();

  // Velocity gradients
  const VariableValue & _velocity_r;
  const VariableValue & _velocity_z;
  const VariableGradient & _grad_velocity_r;
  const VariableGradient & _grad_velocity_z;
  const VariableValue & _pressure;
  const MaterialProperty<Real> & _mu; //converted from AD
};
