//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "InterfaceMaterial.h"

class CZMMaterialLD;
template <> InputParameters validParams<CZMMaterialLD>();
/**
 * Implementation of the non-stateful exponential traction separation law
 * proposed by Salehani, Mohsen Khajeh and Irani, Nilgoon 2018
 **/
class CZMMaterialLD : public InterfaceMaterial {
public:
  CZMMaterialLD(const InputParameters &parameters);

protected:
  virtual void initialSetup() override;

  // this method is responsible for computing the traction and traction rate in
  // the interface coordinate system
  virtual void computeTraction();

  // virtual RankTwoTensor computeTractionDerivatives() override{};

  virtual void initQpStatefulProperties() override;
  virtual void computeQpProperties() override;

  /// normal to the interface
  const MooseArray<Point> &_normals;

  /// number of displacement components
  const unsigned int _ndisp;

  /// the coupled displacement and neighbor displacement values
  ///@{
  std::vector<const VariableValue *> _disp;
  std::vector<const VariableValue *> _disp_neighbor;
  std::vector<const VariableValue *> _disp_old;
  std::vector<const VariableValue *> _disp_neighbor_old;
  ///@}

  /// the displacement jump in global and local coordiante
  ///@{
  MaterialProperty<RealVectorValue> &_displacement_jump_global;
  MaterialProperty<RealVectorValue> &_displacement_jump_global_old;
  MaterialProperty<RealVectorValue> &_displacement_jump;
  const MaterialProperty<RealVectorValue> &_displacement_jump_old;
  MaterialProperty<RealVectorValue> &_displacement_jump_global_dot;
  MaterialProperty<RealVectorValue> &_displacement_jump_dot;
  ///@}

  /// the value of the traction in global and local coordinates
  ///@{
  MaterialProperty<RealVectorValue> &_traction_global;
  MaterialProperty<RealVectorValue> &_traction_global_dot;
  MaterialProperty<RealVectorValue> &_traction;
  MaterialProperty<RealVectorValue> &_traction_dot;
  const MaterialProperty<RealVectorValue> &_traction_global_old;
  const MaterialProperty<RealVectorValue> &_traction_old;
  ///@}

  /// the traction's derivatives wrt the displacement jump in global and local
  /// coordinates
  ///@{
  MaterialProperty<RankTwoTensor> &_traction_derivatives_global;
  MaterialProperty<RankTwoTensor> &_traction_derivatives;
  ///@}

  std::vector<const VariableGradient *> _grad_disp;
  std::vector<const VariableGradient *> _grad_disp_neighbor;

  std::vector<const VariableGradient *> _grad_disp_dot;
  std::vector<const VariableGradient *> _grad_disp_neighbor_dot;

  MaterialProperty<RankTwoTensor> &_F_avg;
  MaterialProperty<RankTwoTensor> &_DF_avg;
  const MaterialProperty<RankTwoTensor> &_F_avg_old;
  MaterialProperty<RankTwoTensor> &_R_avg;
  const MaterialProperty<RankTwoTensor> &_R_avg_old;
  MaterialProperty<RankTwoTensor> &_U_avg;
  MaterialProperty<RankTwoTensor> &_Rdot_avg;
  MaterialProperty<RankTwoTensor> &_L_avg;
  const MaterialProperty<RankTwoTensor> &_L_avg_old;
  MaterialProperty<RealVectorValue> &_n_avg;
  MaterialProperty<Real> &_dsdotdS_avg;
  MaterialProperty<Real> &_dadA_avg;

  // MaterialProperty<RankTwoTensor> &_linear_rot;
  // const MaterialProperty<RankTwoTensor> &_linear_rot_old;
  // MaterialProperty<RankTwoTensor> &_linear_rot_neighbor;
  // const MaterialProperty<RankTwoTensor> &_linear_rot_neighbor_old;
  // MaterialProperty<RankTwoTensor> &_linear_rot_avg;
  // const MaterialProperty<RankTwoTensor> &_linear_rot_avg_old;

  RankTwoTensor _K = RankTwoTensor::Identity();
  const bool _ld;

  // void update_L_df_vorticity(const RankTwoTensor &def_grad,
  //                            const RankTwoTensor &def_grad_old,
  //                            RankTwoTensor &L, RankTwoTensor &df,
  //                            RankTwoTensor &vorticity_inc);

  void computeFandL();

  void PolarDecomposition(const RankTwoTensor &F, RankTwoTensor &R,
                          RankTwoTensor &U) const;

  void computeRdot(const RankTwoTensor &F, const RankTwoTensor &L,
                   const RankTwoTensor &R, const RankTwoTensor &U,
                   RankTwoTensor &Rdot);

  void computeSdot(const RankTwoTensor &L, const RealVectorValue &n,
                   Real &sdot);

  // void computeFAvg(const RankTwoTensor &R, const RankTwoTensor &U,
  //                  const RankTwoTensor &R_neigh, const RankTwoTensor
  //                  &U_neigh, RankTwoTensor &R_avg, RankTwoTensor &U_avg,
  //                  RankTwoTensor &F_avg);
  // void comptueRotationAverage(const RankTwoTensor &R,
  //                             const RankTwoTensor &R_neigh,
  //                             RankTwoTensor &R_avg) const;
  // void mat2quat(const RankTwoTensor &R, std::vector<Real> &q) const;
  //
  // void quat2mat(const std::vector<Real> &quat, RankTwoTensor &M) const;
};
