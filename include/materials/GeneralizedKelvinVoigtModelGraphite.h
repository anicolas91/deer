//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GeneralizedKelvinVoigtBase.h"

class GeneralizedKelvinVoigtModelGraphite;

// template <typename>
// class RankFourTensorTempl;
// typedef RankFourTensorTempl<Real> RankFourTensor;

template <>
InputParameters validParams<GeneralizedKelvinVoigtModelGraphite>();

/**
 * This class is an implementation of a generalized Kelvin-Voigt model
 * with constant mechanical properties. It consists of an arbitrary number
 * of Kelvin-Voigt units associated in series with a single spring, and an
 * optional dashpot (this could represent a Burgers model, for example).
 */
class GeneralizedKelvinVoigtModelGraphite : public GeneralizedKelvinVoigtBase
{
public:
  GeneralizedKelvinVoigtModelGraphite(const InputParameters & parameters);

protected:
  virtual void computeQpViscoelasticProperties();
  virtual void computeQpViscoelasticPropertiesInv();

  // const std::string _base_name;
  /**
   * The elasticity tensor associated with the first spring. This is
   * the true elasticity tensor of the material
   */
  // const MaterialProperty<RankFourTensor> & _elasticity_tensor;
  RankFourTensor _C0;
  /// The elasticity tensor of each subsequent spring
  std::vector<RankFourTensor> _Ci;
  /// The viscosity of each dashpot
  std::vector<Real> _eta_i;

  /// The inverse of the elasticity tensor of the first spring
  RankFourTensor _S0;
  /// The inverse of each subsequent spring elasticity tensor
  std::vector<RankFourTensor> _Si;

  /// The derivative of fluence (actual equation input)
  const Function & _fluence_function_dT;
  const Real _alpha;
  const Real _young_modulus_par;
  const Real _young_modulus_per;
  const Real _poisson_ratio;
  const MaterialProperty<Real> & _EoverE0_par;
  const MaterialProperty<Real> & _EoverE0_per;
};
