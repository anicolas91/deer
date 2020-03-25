//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CZMMaterialLargeDefBase.h"
#include "RotationMatrix.h"

registerMooseObject("DeerApp", CZMMaterialLargeDefBase);

template <> InputParameters validParams<CZMMaterialLargeDefBase>() {
  InputParameters params = validParams<CZMMaterialBase>();
  params.addClassDescription("Base czm material for large deformation");
  params.addParam<bool>("large_kinematics", false,
                        "Use large displacement kinematics.");
  params.addParam<bool>("stagger_kinemtics", true,
                        "Use previous time step geometry");
  params.addParam<Real>("K_interface", 1e3, "stiffness");
  return params;
}

CZMMaterialLargeDefBase::CZMMaterialLargeDefBase(
    const InputParameters &parameters)
    : CZMMaterialBase(parameters), _disp_old(_ndisp),
      _disp_neighbor_old(_ndisp),
      _displacement_jump_global_old(
          declareProperty<RealVectorValue>("displacement_jump_global_old")),

      _traction_avg(declareProperty<RealVectorValue>("traction_avg")),
      _traction_global_avg(
          declareProperty<RealVectorValue>("traction_global_avg")),
      _traction_global_neighbor(
          declareProperty<RealVectorValue>("traction_global_neighbor")),
      _traction_global_avg_old(
          getMaterialPropertyOld<RealVectorValue>("traction_global_avg")),

      _traction_derivatives_avg(
          declareProperty<RankTwoTensor>("traction_derivatives_avg")),
      _traction_derivatives_global_avg(
          declareProperty<RankTwoTensor>("traction_derivatives_global_avg")),
      _traction_derivatives_global_neighbor(declareProperty<RankTwoTensor>(
          "traction_derivatives_global_neighbor")),
      _def_grad(declareProperty<RankTwoTensor>("def_grad")),
      _def_grad_old(declareProperty<RankTwoTensor>("def_grad_old")),
      _def_grad_neighbor(declareProperty<RankTwoTensor>("def_grad_neighbor")),
      _def_grad_neighbor_old(
          declareProperty<RankTwoTensor>("def_grad_neighbor_old")),
      _def_grad_avg(declareProperty<RankTwoTensor>("def_grad_avg")),
      _def_grad_avg_old(declareProperty<RankTwoTensor>("def_grad_avg_old")),

      _n(declareProperty<RealVectorValue>("n_master_czm")),
      _n_neighbor(declareProperty<RealVectorValue>("n_neighbor_czm")),
      _n_avg(declareProperty<RealVectorValue>("n_avg_czm")),
      _n_old(declareProperty<RealVectorValue>("n_master_old__czm")),
      _n_neighbor_old(declareProperty<RealVectorValue>("n_neighbor_czm_old")),
      _n_avg_old(declareProperty<RealVectorValue>("n_avg_old_czm")),

      _dadA(declareProperty<Real>("dadA_master_czm")),
      _dadA_neighbor(declareProperty<Real>("dadA_neighbor_czm")),
      _dadA_avg(declareProperty<Real>("dadA_avg_czm")),
      _dadA_old(declareProperty<Real>("dadA_master_old__czm")),
      _dadA_neighbor_old(declareProperty<Real>("dadA_neighbor_czm_old")),
      _dadA_avg_old(declareProperty<Real>("dadA_avg_old_czm")),

      _neighbor_area(declareProperty<Real>("NeighborArea")),
      _def_grad_neighbor_current(
          declareProperty<RankTwoTensor>("def_grad_neighbor_current")),

      _ld(getParam<bool>("large_kinematics")),
      _K(getParam<Real>("K_interface")) { // Enforce consistency
  if (_ndisp != _mesh.dimension())
    paramError("displacements",
               "Number of displacements must match problem dimension.");
}

void CZMMaterialLargeDefBase::initialSetup() {

  for (unsigned int i = 0; i < _ndisp; i++) {
    _disp_old[i] = &coupledValueOld("displacements", i);
    _disp_neighbor_old[i] = &coupledNeighborValueOld("displacements", i);
    _grad_disp.push_back(getParam<bool>("stagger_kinemtics")
                             ? &coupledGradientOld("displacements", i)
                             : &coupledGradient("displacements", i));

    _grad_disp_neighbor.push_back(
        getParam<bool>("stagger_kinemtics")
            ? &coupledNeighborGradientOld("displacements", i)
            : &coupledNeighborGradient("displacements", i));

    _grad_disp_old.push_back(getParam<bool>("stagger_kinemtics")
                                 ? &coupledGradientOlder("displacements", i)
                                 : &coupledGradientOld("displacements", i));
    _grad_disp_neighbor_old.push_back(
        getParam<bool>("stagger_kinemtics")
            ? &coupledNeighborGradientOlder("displacements", i)
            : &coupledNeighborGradientOld("displacements", i));

    _grad_disp_neighbor_current.push_back(
        &coupledNeighborGradient("displacements", i));
  }

  // All others zero (so this will work naturally for plane strain problems)
  for (unsigned int i = _ndisp; i < 3; i++) {
    _disp_old[i] = &_zero;
    _disp_neighbor_old[i] = &_zero;
    _grad_disp.push_back(&_grad_zero);
    _grad_disp_neighbor.push_back(&_grad_zero);
    _grad_disp_old.push_back(&_grad_zero);
    _grad_disp_neighbor_old.push_back(&_grad_zero);
    _grad_disp_neighbor_current.push_back(&_grad_zero);
  }
}

void CZMMaterialLargeDefBase::initQpStatefulProperties() {

  // _vorticity_inc[_qp].zero();
  // _def_grad[_qp] = RankTwoTensor::Identity();
  // _df[_qp] = RankTwoTensor::Identity();

  // _vorticity_inc_neighbor[_qp].zero();
  // _def_grad_neighbor[_qp] = RankTwoTensor::Identity();
  // _df_neighbor[_qp] = RankTwoTensor::Identity();

  // _vorticity_inc_avg[_qp].zero();
  // _def_grad_avg[_qp] = RankTwoTensor::Identity();
  // _df_avg[_qp] = RankTwoTensor::Identity();

  for (unsigned int i = 0; i < 3; i++)
    _traction_global_avg[_qp](i) = 0;

  // _linear_rot[_qp].zero();
  // _linear_rot_neighbor[_qp].zero();
  // _linear_rot_avg[_qp].zero();
}

void CZMMaterialLargeDefBase::computeQpProperties() {

  // update the displacement jump
  for (unsigned int i = 0; i < 3; i++) {
    _displacement_jump_global[_qp](i) =
        (*_disp_neighbor[i])[_qp] - (*_disp[i])[_qp];
    _displacement_jump_global_old[_qp](i) =
        (*_disp_neighbor_old[i])[_qp] - (*_disp_old[i])[_qp];
  }
  RealVectorValue _displacement_jump_increment_global =
      _displacement_jump_global[_qp] - _displacement_jump_global_old[_qp];

  _def_grad[_qp] = (RankTwoTensor::Identity() +
                    RankTwoTensor((*_grad_disp[0])[_qp], (*_grad_disp[1])[_qp],
                                  (*_grad_disp[2])[_qp]));

  _def_grad_neighbor[_qp] = (RankTwoTensor::Identity() +
                             RankTwoTensor((*_grad_disp_neighbor[0])[_qp],
                                           (*_grad_disp_neighbor[1])[_qp],
                                           (*_grad_disp_neighbor[2])[_qp]));

  _def_grad_avg[_qp] = (_def_grad[_qp] + _def_grad_neighbor[_qp]) * 0.5;

  _def_grad_old[_qp] =
      (RankTwoTensor::Identity() + RankTwoTensor((*_grad_disp_old[0])[_qp],
                                                 (*_grad_disp_old[1])[_qp],
                                                 (*_grad_disp_old[2])[_qp]));

  _def_grad_neighbor_old[_qp] =
      (RankTwoTensor::Identity() +
       RankTwoTensor((*_grad_disp_neighbor_old[0])[_qp],
                     (*_grad_disp_neighbor_old[1])[_qp],
                     (*_grad_disp_neighbor_old[2])[_qp]));

  _def_grad_avg_old[_qp] =
      (_def_grad_old[_qp] + _def_grad_neighbor_old[_qp]) * 0.5;

  _def_grad_neighbor_current[_qp] =
      (RankTwoTensor::Identity() +
       RankTwoTensor((*_grad_disp_neighbor_current[0])[_qp],
                     (*_grad_disp_neighbor_current[1])[_qp],
                     (*_grad_disp_neighbor_current[2])[_qp]));

  // compute kinematics quantities
  RankTwoTensor R, Rneighbor, Ravg, R_old, Rneighbor_old, Ravg_old;
  computeRotation(_def_grad[_qp], R);
  computeRotation(_def_grad_neighbor[_qp], Rneighbor);
  computeRotation(_def_grad_avg[_qp], Ravg);

  // computeRotation(_def_grad_old[_qp], R_old);
  // computeRotation(_def_grad_neighbor_old[_qp], Rneighbor_old);
  // computeRotation(_def_grad_avg_old[_qp], Ravg_old);

  RankTwoTensor U, Uneighbor, Uavg; // U_old, Uneighbor_old, Uavg_old;

  U = R.transpose() * _def_grad[_qp];
  Uneighbor = Rneighbor.transpose() * _def_grad_neighbor[_qp];
  Uavg = Ravg.transpose() * _def_grad_avg[_qp];

  // U_old = R_old.inverse() * _def_grad_old[_qp];
  // Uneighbor_old = Rneighbor_old.inverse() * _def_grad_neighbor_old[_qp];
  // Uavg_old = Ravg_old.inverse() * _def_grad_avg_old[_qp];

  _n[_qp] = _def_grad[_qp] * _normals[_qp];
  _n_neighbor[_qp] = -_def_grad_neighbor[_qp] * _normals[_qp];
  _n_avg[_qp] = _def_grad_avg[_qp] * _normals[_qp];

  RealTensorValue RotationGlobalToLocal = RotationMatrix::rotVec1ToVec2(
      _n_avg[_qp] / (_n_avg[_qp].norm()), RealVectorValue(1, 0, 0));

  // find the old traction in the new defromed average plane in local
  // coordinates
  RealVectorValue traction_avg_old =
      RotationGlobalToLocal *
      (_def_grad[_qp] *
       (_def_grad_avg_old[_qp].inverse() * _traction_global_avg_old[_qp]));

  // rotate the displacement jump to the local coordiante system
  _displacement_jump[_qp] =
      RotationGlobalToLocal * _displacement_jump_global[_qp];

  _traction_avg[_qp] = _K * _displacement_jump[_qp];
  std::cout << "_K * _displacement_jump[_qp]; " << _K * _displacement_jump[_qp]
            << std::endl;
  for (unsigned int i = 1; i < 3; i++)
    _traction_avg[_qp](i) = 0;

  _traction_global_avg[_qp] =
      RotationGlobalToLocal.transpose() * _traction_avg[_qp];

  _traction_derivatives_avg[_qp] = _K * RankTwoTensor::Identity();
  _traction_derivatives_global_avg[_qp] = _traction_derivatives_avg[_qp];
  _traction_derivatives_global_avg[_qp].rotate(
      RotationGlobalToLocal.transpose());

  _traction_global[_qp] = (U.inverse().transpose()) * (Uavg.transpose()) *
                          _traction_global_avg[_qp];
  _traction_global_neighbor[_qp] = (Uneighbor.inverse().transpose()) *
                                   (Uavg.transpose()) *
                                   _traction_global_avg[_qp];

  _traction_derivatives[_qp] = (U.inverse().transpose()) * (Uavg.transpose()) *
                               _traction_derivatives_global_avg[_qp];
  _traction_derivatives_global_neighbor[_qp] =
      (Uneighbor.inverse().transpose()) * (Uavg.transpose()) *
      _traction_derivatives_global_avg[_qp];

  _dadA[_qp] =
      _def_grad[_qp].det() *
      (((_def_grad[_qp].inverse()).transpose() * _normals[_qp]).norm());

  _dadA_neighbor[_qp] =
      _def_grad_neighbor[_qp].det() *
      (((_def_grad_neighbor[_qp].inverse()).transpose() * _normals[_qp])
           .norm());
  _dadA_avg[_qp] =
      _def_grad_avg[_qp].det() *
      (((_def_grad_avg[_qp].inverse()).transpose() * _normals[_qp]).norm());

  _neighbor_area[_qp] =
      _def_grad_neighbor_current[_qp].det() *
      (((_def_grad_neighbor_current[_qp].inverse()).transpose() * _normals[_qp])
           .norm()) *
      _JxW[_qp];

  _n[_qp] = (_def_grad[_qp].inverse()).transpose() * _normals[_qp];
  _n_neighbor[_qp] =
      -(_def_grad_neighbor[_qp].inverse()).transpose() * _normals[_qp];
  _n_avg[_qp] = (_def_grad_avg[_qp].inverse()).transpose() * _normals[_qp];

  _n[_qp] /= _n[_qp].norm();
  _n_neighbor[_qp] /= _n_neighbor[_qp].norm();
  _n_avg[_qp] /= _n_avg[_qp].norm();
  //
  // _n_old[_qp] = R_old * _normals[_qp];
  // _n_neighbor_old[_qp] = -Rneighbor_old * _normals[_qp];
  // _n_avg_old[_qp] = Ravg_old * _normals[_qp];
  //
  // _dadA_old[_qp] = _n_old[_qp].norm();
  // _dadA_neighbor_old[_qp] = _n_neighbor_old[_qp].norm();
  // _dadA_avg_old[_qp] = _n_avg_old[_qp].norm();
  //
  // _n_old[_qp] /= _n_old[_qp].norm();
  // _n_neighbor_old[_qp] /= _n_neighbor_old[_qp].norm();
  // _n_avg_old[_qp] /= _n_avg_old[_qp].norm();
  //
  // if (_qp == 0) {
  std::cout << "normals\n" << _normals[_qp] << std::endl;
  std::cout << "JxW\n" << _JxW[_qp] << std::endl;
  std::cout << "n\n" << _n[_qp] << std::endl;
  std::cout << "n_neighbor\n" << _n_neighbor[_qp] << std::endl;
  std::cout << "n_avg\n" << _n_avg[_qp] << std::endl;

  std::cout << "J\n" << _def_grad[_qp].det() << std::endl;
  std::cout << "J_neighbor\n" << _def_grad_neighbor[_qp].det() << std::endl;
  std::cout << "J_avg\n" << _def_grad_avg[_qp].det() << std::endl;

  std::cout << "dadA\n" << _dadA[_qp] << std::endl;
  std::cout << "dadA_neighbor\n" << _dadA_neighbor[_qp] << std::endl;
  std::cout << "dadA_avg\n" << _dadA_avg[_qp] << std::endl;
  std::cout << "_neighbor_area\n" << _neighbor_area[_qp] << std::endl;
  std::cout << "_traction\n" << _traction[_qp] << std::endl;
  std::cout << "_traction_global_avg\n"
            << _traction_global_avg[_qp] << std::endl;
  std::cout << "_traction_global\n" << _traction_global[_qp] << std::endl;
  std::cout << "_traction_global_neighbor\n"
            << _traction_global_neighbor[_qp] << std::endl
            << std::endl
            << std::endl;
  // }
}

void CZMMaterialLargeDefBase::computeRotation(const RankTwoTensor &F,
                                              RankTwoTensor &R) const {

  RankTwoTensor R_old = F;
  R = 0.5 * (R_old + (R_old.inverse()).transpose());
  while ((R - R_old).L2norm() > 1e-15) {
    R_old = R;
    R = 0.5 * (R_old + (R_old.inverse()).transpose());
  }
}
