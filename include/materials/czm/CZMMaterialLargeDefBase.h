//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "CZMMaterialBase.h"

class CZMMaterialLargeDefBase;
template <> InputParameters validParams<CZMMaterialLargeDefBase>();
/**
 * Implementation of the non-stateful exponential traction separation law
 * proposed by Salehani, Mohsen Khajeh and Irani, Nilgoon 2018
 **/
class CZMMaterialLargeDefBase : public CZMMaterialBase {
public:
  CZMMaterialLargeDefBase(const InputParameters &parameters);

protected:
  virtual void initialSetup() override;
  virtual RealVectorValue computeTraction() override{};

  virtual RankTwoTensor computeTractionDerivatives() override{};

  virtual void initQpStatefulProperties() override;
  virtual void computeQpProperties() override;

  /// the coupled displacement and neighbor displacement values
  ///@{
  std::vector<const VariableValue *> _disp_old;
  std::vector<const VariableValue *> _disp_neighbor_old;
  ///@}

  MaterialProperty<RealVectorValue> &_displacement_jump_global_old;

  MaterialProperty<RealVectorValue> &_traction_avg;
  MaterialProperty<RealVectorValue> &_traction_global_avg;
  MaterialProperty<RealVectorValue> &_traction_global_neighbor;

  const MaterialProperty<RealVectorValue> &_traction_global_avg_old;

  /// the traction's derivatives wrt the displacement jump in global and local
  /// coordinates
  ///@{
  MaterialProperty<RankTwoTensor> &_traction_derivatives_avg;
  MaterialProperty<RankTwoTensor> &_traction_derivatives_global_avg;

  ///@}

  /// the traction's derivatives wrt the displacement jump in global and local
  /// coordinates for each surface
  ///@{
  MaterialProperty<RankTwoTensor> &_traction_derivatives_global_neighbor;
  ///@}

  std::vector<const VariableGradient *> _grad_disp;
  std::vector<const VariableGradient *> _grad_disp_old;
  std::vector<const VariableGradient *> _grad_disp_neighbor;
  std::vector<const VariableGradient *> _grad_disp_neighbor_old;

  MaterialProperty<RankTwoTensor> &_def_grad;
  MaterialProperty<RankTwoTensor> &_def_grad_old;
  MaterialProperty<RankTwoTensor> &_def_grad_neighbor;
  MaterialProperty<RankTwoTensor> &_def_grad_neighbor_old;
  MaterialProperty<RankTwoTensor> &_def_grad_avg;
  MaterialProperty<RankTwoTensor> &_def_grad_avg_old;

  MaterialProperty<RealVectorValue> &_n;
  MaterialProperty<RealVectorValue> &_n_neighbor;
  MaterialProperty<RealVectorValue> &_n_avg;
  MaterialProperty<RealVectorValue> &_n_old;
  MaterialProperty<RealVectorValue> &_n_neighbor_old;
  MaterialProperty<RealVectorValue> &_n_avg_old;

  MaterialProperty<Real> &_dadA;
  MaterialProperty<Real> &_dadA_neighbor;
  MaterialProperty<Real> &_dadA_avg;
  MaterialProperty<Real> &_dadA_old;
  MaterialProperty<Real> &_dadA_neighbor_old;
  MaterialProperty<Real> &_dadA_avg_old;

  MaterialProperty<Real> &_neighbor_area;
  std::vector<const VariableGradient *> _grad_disp_neighbor_current;
  MaterialProperty<RankTwoTensor> &_def_grad_neighbor_current;
  // MaterialProperty<RankTwoTensor> &_linear_rot;
  // const MaterialProperty<RankTwoTensor> &_linear_rot_old;
  // MaterialProperty<RankTwoTensor> &_linear_rot_neighbor;
  // const MaterialProperty<RankTwoTensor> &_linear_rot_neighbor_old;
  // MaterialProperty<RankTwoTensor> &_linear_rot_avg;
  // const MaterialProperty<RankTwoTensor> &_linear_rot_avg_old;

  const bool _ld;
  const Real _K;

  // void update_L_df_vorticity(const RankTwoTensor &def_grad,
  //                            const RankTwoTensor &def_grad_old,
  //                            RankTwoTensor &L, RankTwoTensor &df,
  //                            RankTwoTensor &vorticity_inc);

  void computeRotation(const RankTwoTensor &F, RankTwoTensor &R) const;
};
