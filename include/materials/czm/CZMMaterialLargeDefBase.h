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
  virtual RealVectorValue computeTraction() override{};

  virtual RankTwoTensor computeTractionDerivatives() override{};

  virtual void initQpStatefulProperties() override;
  virtual void computeQpProperties() override;

  RankTwoTensor computeF(std::vector<const VariableGradient *> g_disp) const;

  std::vector<const VariableGradient *> _grad_disp;
  std::vector<const VariableGradient *> _grad_disp_old;

  std::vector<const VariableGradient *> _grad_disp_neighbor;
  std::vector<const VariableGradient *> _grad_disp_neighbor_old;

  MaterialProperty<RankTwoTensor> &_vorticity_inc;
  MaterialProperty<RankTwoTensor> &_def_grad;
  const MaterialProperty<RankTwoTensor> &_def_grad_old;
  MaterialProperty<RankTwoTensor> &_df;

  MaterialProperty<RankTwoTensor> &_vorticity_inc_neighbor;
  MaterialProperty<RankTwoTensor> &_def_grad_neighbor;
  const MaterialProperty<RankTwoTensor> &_def_grad_neighbor_old;
  MaterialProperty<RankTwoTensor> &_df_neighbor;

  MaterialProperty<RankTwoTensor> &_vorticity_inc_avg;
  MaterialProperty<RankTwoTensor> &_def_grad_avg;
  const MaterialProperty<RankTwoTensor> &_def_grad_avg_old;
  MaterialProperty<RankTwoTensor> &_df_avg;

  // MaterialProperty<RankTwoTensor> &_F;
  // MaterialProperty<RankTwoTensor> &_F_neighbor;
  // MaterialProperty<RankTwoTensor> &_F_old;
  // MaterialProperty<RankTwoTensor> &_F_neighbor_old;
  // MaterialProperty<RankTwoTensor> &_F_avg;
  // MaterialProperty<RankTwoTensor> &_F_avg_old;

  // MaterialProperty<RealVectorValue> &_n;
  // MaterialProperty<RealVectorValue> &_n_neighbor;
  MaterialProperty<RealVectorValue> &_n_avg;
  // MaterialProperty<RealVectorValue> &_n_old;
  // MaterialProperty<RealVectorValue> &_n_neighbor_old;
  const MaterialProperty<RealVectorValue> &_n_avg_old;

  MaterialProperty<RealVectorValue> &_n_zero;
  MaterialProperty<RealVectorValue> &_n_zero_neighbor;
  const MaterialProperty<RealVectorValue> &_n_zero_old;
  const MaterialProperty<RealVectorValue> &_n_zero_neighbor_old;
  // const RankTwoTensor _I(RankTwoTensor::initIdentity());

  const bool _ld;

  void update_L_df_vorticity(const RankTwoTensor &def_grad,
                             const RankTwoTensor &def_grad_old,
                             RankTwoTensor &L, RankTwoTensor &df,
                             RankTwoTensor &vorticity_inc);
};
