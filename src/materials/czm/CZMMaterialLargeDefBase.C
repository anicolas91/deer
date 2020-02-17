//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CZMMaterialLargeDefBase.h"

registerMooseObject("DeerApp", CZMMaterialLargeDefBase);

template <> InputParameters validParams<CZMMaterialLargeDefBase>() {
  InputParameters params = validParams<CZMMaterialBase>();
  params.addClassDescription("Base czm material for large deformation");
  params.addParam<bool>("large_kinematics", false,
                        "Use large displacement kinematics.");
  return params;
}

CZMMaterialLargeDefBase::CZMMaterialLargeDefBase(
    const InputParameters &parameters)
    : CZMMaterialBase(parameters),
      // _strain_inc(declareProperty<RankTwoTensor>("strain_inc")),
      // _mechanical_strain_inc(
      //     declareProperty<RankTwoTensor>("mechanical_strain_inc")),
      _vorticity_inc(declareProperty<RankTwoTensor>("vorticity_inc")),
      _def_grad(declareProperty<RankTwoTensor>("def_grad")),
      _def_grad_old(getMaterialPropertyOld<RankTwoTensor>("def_grad")),
      _df(declareProperty<RankTwoTensor>("df")),

      _vorticity_inc_neighbor(
          declareProperty<RankTwoTensor>("vorticity_inc_neighbor")),
      _def_grad_neighbor(declareProperty<RankTwoTensor>("def_grad_neighbor")),
      _def_grad_neighbor_old(
          getMaterialPropertyOld<RankTwoTensor>("def_grad_neighbor")),
      _df_neighbor(declareProperty<RankTwoTensor>("df_neighbor")),

      _vorticity_inc_avg(declareProperty<RankTwoTensor>("vorticity_inc_avg")),
      _def_grad_avg(declareProperty<RankTwoTensor>("def_grad_avg")),
      _def_grad_avg_old(getMaterialPropertyOld<RankTwoTensor>("def_grad_avg")),
      _df_avg(declareProperty<RankTwoTensor>("df_avg")),

      // _F(declareProperty<RankTwoTensor>("F_master_czm")),
      // _F_neighbor(declareProperty<RankTwoTensor>("F_neighbor_czm")),
      // _F_old(declareProperty<RankTwoTensor>("F_master_czm")),
      // _F_neighbor_old(declareProperty<RankTwoTensor>("F_neighbor_czm")),
      // _F_avg(declareProperty<RankTwoTensor>("F_avg_czm")),
      // _F_avg_old(declareProperty<RankTwoTensor>("F_avg_old_czm")),
      // _n(declareProperty<RealVectorValue>("n_master_czm")),
      // _n_neighbor(declareProperty<RealVectorValue>("n_neighbor_czm")),
      _n_avg(declareProperty<RealVectorValue>("n_avg_czm")),
      // _n_old(declareProperty<RealVectorValue>("n_master_czm_old")),
      // _n_neighbor_old(declareProperty<RealVectorValue>("n_neighbor_czm_old")),
      _n_avg_old(getMaterialPropertyOld<RealVectorValue>("n_avg_czm")),
      _n_zero(declareProperty<RealVectorValue>("n_zero_master_czm")),
      _n_zero_neighbor(declareProperty<RealVectorValue>("n_zero_neighbor_czm")),
      _n_zero_old(getMaterialPropertyOld<RealVectorValue>("n_zero_master_czm")),
      _n_zero_neighbor_old(
          getMaterialPropertyOld<RealVectorValue>("n_zero_slave_czm")),
      _ld(getParam<bool>("large_kinematics")) { // Enforce consistency
  if (_ndisp != _mesh.dimension()) {
    paramError("displacements",
               "Number of displacements must match problem dimension.");
  }

  for (unsigned int i = 0; i < _ndisp; i++) {
    _grad_disp[i] = &coupledGradient("displacements", i);
    _grad_disp_neighbor[i] = &coupledNeighborGradient("displacements", i);
    _grad_disp_old[i] = &coupledGradientOld("displacements", i);
    _grad_disp_neighbor_old[i] =
        &coupledNeighborGradientOld("displacements", i);
  }

  // All others zero (so this will work naturally for plane strain problems)
  for (unsigned int i = _ndisp; i < 3; i++) {
    _grad_disp[i] = &_grad_zero;
    _grad_disp_neighbor[i] = &_grad_zero;
    _grad_disp_old[i] = &_grad_zero;
    _grad_disp_neighbor_old[i] = &_grad_zero;
  }
}

void CZMMaterialLargeDefBase::initQpStatefulProperties() {
  _vorticity_inc[_qp].zero();
  _def_grad[_qp] = RankTwoTensor::Identity();
  _df[_qp] = RankTwoTensor::Identity();

  _vorticity_inc_neighbor[_qp].zero();
  _def_grad_neighbor[_qp] = RankTwoTensor::Identity();
  _df_neighbor[_qp] = RankTwoTensor::Identity();

  _vorticity_inc_avg[_qp].zero();
  _def_grad_avg[_qp] = RankTwoTensor::Identity();
  _df_avg[_qp] = RankTwoTensor::Identity();

  _n_avg[_qp] = _normals[_qp];
  _n_zero[_qp] = _normals[_qp];
  _n_zero_neighbor[_qp] -= _n_zero[_qp];
}

void CZMMaterialLargeDefBase::computeQpProperties() {

  _def_grad[_qp] = (RankTwoTensor::Identity() +
                    RankTwoTensor((*_grad_disp[0])[_qp], (*_grad_disp[1])[_qp],
                                  (*_grad_disp[2])[_qp]));

  _def_grad_neighbor[_qp] = (RankTwoTensor::Identity() +
                             RankTwoTensor((*_grad_disp_neighbor[0])[_qp],
                                           (*_grad_disp_neighbor[1])[_qp],
                                           (*_grad_disp_neighbor[2])[_qp]));

  _def_grad_avg[_qp] = (_def_grad[_qp] + _def_grad_neighbor[_qp]) * 0.5;

  _n_zero[_qp] = _n_zero_old[_qp];
  _n_zero_neighbor[_qp] = _n_zero_neighbor_old[_qp];
  // _F[_qp] = computeF(_grad_disp);
  // _F_old[_qp] = computeF(_grad_disp_old);
  // _F_neighbor[_qp] = computeF(_grad_disp_neighbor);
  // _F_neighbor_old[_qp] = computeF(_grad_disp_neighbor_old);
  // _F_avg[_qp] = (_F[_qp] + _F_neighbor[_qp]) * 0.5;
  // _F_avg_old[_qp] = (_F_old[_qp] + _F_neighbor_old[_qp]) * 0.5;

  RankTwoTensor L, L_neighbor, L_avg;
  update_L_df_vorticity(_def_grad[_qp], _def_grad_old[_qp], L, _df[_qp],
                        _vorticity_inc[_qp]);
  update_L_df_vorticity(_def_grad_neighbor[_qp], _def_grad_neighbor_old[_qp],
                        L_neighbor, _df_neighbor[_qp],
                        _vorticity_inc_neighbor[_qp]);
  update_L_df_vorticity(_def_grad_avg[_qp], _def_grad_avg_old[_qp], L_avg,
                        _df_avg[_qp], _vorticity_inc_avg[_qp]);
}

// RankTwoTensor CZMMaterialLargeDefBase::computeF(
//     std::vector<const VariableGradient *> g_disp) const {
//   RankTwoTensor F = (RankTwoTensor::Identity() +
//                      RankTwoTensor((*_grad_disp[0])[_qp],
//                      (*_grad_disp[1])[_qp],
//                                    (*_grad_disp[2])[_qp]));
//   return F;
// }

void CZMMaterialLargeDefBase::update_L_df_vorticity(
    const RankTwoTensor &def_grad, const RankTwoTensor &def_grad_old,
    RankTwoTensor &L, RankTwoTensor &df, RankTwoTensor &vorticity_inc) {

  if (_ld) {
    L = RankTwoTensor::Identity() - def_grad_old * def_grad.inverse();
    df = -L + RankTwoTensor::Identity();
  } else {
    L = def_grad_old - def_grad.inverse();
    df = RankTwoTensor::Identity();
  }
  vorticity_inc = (L - L.transpose()) / 2.0;
}
