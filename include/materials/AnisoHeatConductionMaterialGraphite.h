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
class AnisoHeatConductionMaterialGraphite;
class Function;

template <>
InputParameters validParams<AnisoHeatConductionMaterialGraphite>();

/**
 * Simple material with constant properties.
 */
class AnisoHeatConductionMaterialGraphite : public Material
{
public:
  AnisoHeatConductionMaterialGraphite(const InputParameters & parameters);

protected:
  virtual void computeProperties();

  const bool _has_temp;
  const VariableValue & _temperature;

  const Real _my_thermal_conductivity_x;
  const Real _my_thermal_conductivity_y;
  const Real _my_thermal_conductivity_z;
  const Real _my_specific_heat;

  MaterialProperty<Real> & _thermal_conductivity_x;
  MaterialProperty<Real> & _thermal_conductivity_x_dT;
  const Function * _thermal_conductivity_x_temperature_function;
  MaterialProperty<Real> & _thermal_conductivity_y;
  MaterialProperty<Real> & _thermal_conductivity_y_dT;
  const Function * _thermal_conductivity_y_temperature_function;
  MaterialProperty<Real> & _thermal_conductivity_z;
  MaterialProperty<Real> & _thermal_conductivity_z_dT;
  const Function * _thermal_conductivity_z_temperature_function;

  MaterialProperty<Real> & _specific_heat;
  const Function * _specific_heat_temperature_function;
};
