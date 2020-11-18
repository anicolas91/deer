//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ComputeRotatedElasticityTensorBase.h"

class ComputeGraphiteElasticityTensor;

template <>
InputParameters validParams<ComputeGraphiteElasticityTensor>();

/**
 * ComputeGraphiteElasticityTensor defines an elasticity tensor material object with a given base name and some input values.
 */
class ComputeGraphiteElasticityTensor : public ComputeRotatedElasticityTensorBase
{
public:
  ComputeGraphiteElasticityTensor(const InputParameters & parameters);

protected:
  const MaterialProperty<Real> & _prefactor_par;
  const MaterialProperty<Real> & _prefactor_per;
  const Real _Epar;
  const Real _Eper;
  const Real _nu;

  virtual void computeQpElasticityTensor() override;

  /// Individual material information
  RankFourTensor _Cijkl;
};
