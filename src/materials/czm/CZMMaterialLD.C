//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CZMMaterialLD.h"
#include "RotationMatrix.h"

registerMooseObject("DeerApp", CZMMaterialLD);

template <> InputParameters validParams<CZMMaterialLD>() {
  InputParameters params = InterfaceMaterial::validParams();
  params.addClassDescription(
      "Base class for cohesive zone material models for rate depndent");
  params.addRequiredCoupledVar(
      "displacements",
      "The string of displacements suitable for the problem statement");
  params.addClassDescription("Base czm material for large deformation");
  params.addParam<bool>("large_kinematics", true,
                        "Use large displacement kinematics.");
  params.addParam<Real>("E", 1e3, "opening stiffness");
  params.addParam<Real>("G", 1e2, "shear modulus");
  return params;
}

CZMMaterialLD::CZMMaterialLD(const InputParameters &parameters)
    : InterfaceMaterial(parameters), _normals(_assembly.normals()),
      _ndisp(coupledComponents("displacements")), _disp(3), _disp_neighbor(3),
      _disp_old(3), _disp_neighbor_old(3),
      _displacement_jump_global(
          declareProperty<RealVectorValue>("displacement_jump_global")),
      _displacement_jump_global_old(
          declareProperty<RealVectorValue>("displacement_jump_global_old")),
      _displacement_jump(declareProperty<RealVectorValue>("displacement_jump")),
      _displacement_jump_old(
          getMaterialPropertyOld<RealVectorValue>("displacement_jump")),
      _displacement_jump_global_dot(
          declareProperty<RealVectorValue>("displacement_jump_global_dot")),
      _displacement_jump_dot(
          declareProperty<RealVectorValue>("displacement_jump_dot")),

      _traction_global(declareProperty<RealVectorValue>("traction_global")),
      _traction_global_dot(
          declareProperty<RealVectorValue>("traction_global_dot")),
      _traction(declareProperty<RealVectorValue>("traction")),
      _traction_dot(declareProperty<RealVectorValue>("traction_dot")),
      _traction_global_old(
          getMaterialPropertyOld<RealVectorValue>("traction_global")),
      _traction_old(getMaterialPropertyOld<RealVectorValue>("traction")),
      _traction_derivatives_global(
          declareProperty<RankTwoTensor>("traction_derivatives_global")),
      _traction_derivatives(
          declareProperty<RankTwoTensor>("traction_derivatives")),

      _F_avg(declareProperty<RankTwoTensor>("F_avg")),
      _DF_avg(declareProperty<RankTwoTensor>("DF_avg")),
      _F_avg_old(getMaterialPropertyOld<RankTwoTensor>("F_avg")),
      _R_avg(declareProperty<RankTwoTensor>("R_avg")),
      _R_avg_old(getMaterialPropertyOld<RankTwoTensor>("R_avg")),
      _U_avg(declareProperty<RankTwoTensor>("U_avg")),
      _Rdot_avg(declareProperty<RankTwoTensor>("Rdot_avg")),
      _L_avg(declareProperty<RankTwoTensor>("L_avg")),
      _L_avg_old(getMaterialPropertyOld<RankTwoTensor>("L_avg")),
      _n_avg(declareProperty<RealVectorValue>("n_avg")),
      _dsdotdS_avg(declareProperty<Real>("dsdotdS_avg")),
      _dadA_avg(declareProperty<Real>("dadA_avg")),
      // _K({getParam<Real>("E"), getParam<Real>("G"), getParam<Real>("G")}),
      _ld(getParam<bool>("large_kinematics")) { // Enforce consistency
  if (_ndisp != _mesh.dimension())
    paramError("displacements",
               "Number of displacements must match problem dimension.");
}

void CZMMaterialLD::initialSetup() {
  _K(0, 0) = getParam<Real>("E");
  _K(1, 1) = getParam<Real>("G");
  _K(2, 2) = getParam<Real>("G");
  // initializing the displacement vectors
  for (unsigned int i = 0; i < _ndisp; ++i) {
    _disp[i] = &coupledValue("displacements", i);
    _disp_neighbor[i] = &coupledNeighborValue("displacements", i);
    _disp_old[i] = &coupledValueOld("displacements", i);
    _disp_neighbor_old[i] = &coupledNeighborValueOld("displacements", i);

    _grad_disp.push_back(&coupledGradient("displacements", i));
    _grad_disp_neighbor.push_back(&coupledNeighborGradient("displacements", i));
    _grad_disp_dot.push_back(&coupledGradientDot("displacements", i));
    _grad_disp_neighbor_dot.push_back(
        &coupledNeighborGradientDot("displacements", i));
  }

  // All others zero (so this will work naturally for 2D and 1D problems)
  for (unsigned int i = _ndisp; i < 3; i++) {
    _disp[i] = &_zero;
    _disp_neighbor[i] = &_zero;
    _disp_old[i] = &_zero;
    _disp_neighbor_old[i] = &_zero;
    _grad_disp.push_back(&_grad_zero);
    _grad_disp_neighbor.push_back(&_grad_zero);
    _grad_disp_dot.push_back(&_grad_zero);
    _grad_disp_neighbor_dot.push_back(&_grad_zero);
  }
}

void CZMMaterialLD::initQpStatefulProperties() {

  _traction_global[_qp] = 0;
  _traction[_qp] = 0;
  _displacement_jump[_qp] = 0;
  _F_avg[_qp] = RankTwoTensor::Identity();
  _R_avg[_qp] = RankTwoTensor::Identity();
  _L_avg[_qp] = RankTwoTensor::initNone;
}

void CZMMaterialLD::computeQpProperties() {

  // update the displacement jump
  for (unsigned int i = 0; i < 3; i++) {
    _displacement_jump_global[_qp](i) =
        (*_disp_neighbor[i])[_qp] - (*_disp[i])[_qp];
    _displacement_jump_global_old[_qp](i) =
        (*_disp_neighbor_old[i])[_qp] - (*_disp_old[i])[_qp];
  }
  _displacement_jump_global_dot[_qp] =
      (_displacement_jump_global[_qp] - _displacement_jump_global_old[_qp]);

  const RealTensorValue Q0 =
      RotationMatrix::rotVec1ToVec2(_normals[_qp], RealVectorValue(1, 0, 0));

  RankTwoTensor DR, DF, DRF, DUF, Rdot;
  if (_ld) {
    computeFandL();
    PolarDecomposition(_F_avg[_qp], _R_avg[_qp], _U_avg[_qp]);
    DF = _F_avg[_qp] * _F_avg_old[_qp].inverse();
    DR = _R_avg[_qp] * _R_avg_old[_qp].inverse();
    PolarDecomposition(DF, DRF, DUF);
    computeRdot(_F_avg[_qp], _L_avg[_qp], _R_avg[_qp], _U_avg[_qp], Rdot);
    _n_avg[_qp] = _R_avg[_qp] * _normals[_qp];
    _dsdotdS_avg[_qp] = 0.;
    _dadA_avg[_qp] = 1.;

  } else {

    _n_avg[_qp] = _normals[_qp];
    _dsdotdS_avg[_qp] = 0.;
    _dadA_avg[_qp] = 1.;
    _Rdot_avg[_qp] = RankTwoTensor::initNone;
    _R_avg[_qp] = RankTwoTensor::Identity();
  }

  _traction_global_dot[_qp] =

      Q0.transpose() * _K * Q0 * DR.transpose() *
          _displacement_jump_global[_qp] +
      Q0.transpose() * _K * Q0 * _R_avg[_qp].transpose() *
          _displacement_jump_global_dot[_qp];

  _traction_global[_qp] = _traction_global[_qp] + _traction_global_dot[_qp];

  std::cout << "\n\n\n";
  std::cout << "_traction_global; " << _traction_global[_qp] << std::endl;
  std::cout << "_traction_global_dot; " << _traction_global_dot[_qp]
            << std::endl;
  std::cout << "_traction_global_old; " << _traction_global_old[_qp]
            << std::endl;
  std::cout << "_traction; " << _traction[_qp] << std::endl;
  std::cout << "_traction_dot; " << _traction_dot[_qp] << std::endl;
  std::cout << "DR; " << DR << std::endl;
  std::cout << "DRF; " << DRF << std::endl;
  std::cout << "Rdot; " << Rdot << std::endl;
  std::cout << "Q0; " << Q0 << std::endl;
  std::cout << "_R_avg; " << _R_avg[_qp] << std::endl;
  std::cout << "n_avg; " << _n_avg[_qp] << std::endl;
  std::cout << "_displacement_jump; " << _displacement_jump[_qp] << std::endl;
  std::cout << "_displacement_jump_global; " << _displacement_jump_global[_qp]
            << std::endl;
  std::cout << "_displacement_jump_dot; " << _displacement_jump_dot[_qp]
            << std::endl;
  std::cout << "_displacement_jump_global_dot; "
            << _displacement_jump_global_dot[_qp] << std::endl;
  // _traction_derivatives[_qp] = (U.inverse().transpose()) * (Uavg.transpose())
  // *
  //                              _traction_derivatives_global_avg[_qp];
  // _traction_derivatives_global_neighbor[_qp] =
  //     (Uneighbor.inverse().transpose()) * (Uavg.transpose()) *
  //     _traction_derivatives_global_avg[_qp];

  // _n_avg[_qp] = (_def_grad_avg[_qp].inverse()).transpose() * _normals[_qp];
  // _n_avg[_qp] /= _n_avg[_qp].norm();
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
  // std::cout << "F\n" << _def_grad[_qp] << std::endl;
  // std::cout << "F_neigh\n" << _def_grad_neighbor[_qp] << std::endl;
  //
  // std::cout << "F_avg\n" << _def_grad_avg[_qp] << std::endl;
  // std::cout << "R_avg\n" << R_avg << std::endl;
  // std::cout << "U_avg\n" << U_avg << std::endl;
  //
  // std::cout << "normals\n" << _normals[_qp] << std::endl;
  // std::cout << "JxW\n" << _JxW[_qp] << std::endl;
  // std::cout << "n_avg\n" << _n_avg[_qp] << std::endl;
  // std::cout << "_displacement_jump\n" << _displacement_jump[_qp] <<
  // std::endl; std::cout << "_displacement_jump_global\n"
  //           << _displacement_jump_global[_qp] << std::endl;
  // std::cout << "J_avg\n" << _def_grad_avg[_qp].det() << std::endl;
  //
  // std::cout << "dadA_avg\n" << _dadA_avg[_qp] << std::endl;
  //
  // std::cout << "_traction\n" << _traction[_qp] << std::endl;
  // std::cout << "_traction_global_avg\n"
  //           << _traction_global_avg[_qp] << std::endl;
  // std::cout << "_traction_global\n" << _traction_global[_qp] << std::endl;

  // }
}

void CZMMaterialLD::computeFandL() {

  RankTwoTensor F = (RankTwoTensor::Identity() +
                     RankTwoTensor((*_grad_disp[0])[_qp], (*_grad_disp[1])[_qp],
                                   (*_grad_disp[2])[_qp]));
  RankTwoTensor F_neighbor = (RankTwoTensor::Identity() +
                              RankTwoTensor((*_grad_disp_neighbor[0])[_qp],
                                            (*_grad_disp_neighbor[1])[_qp],
                                            (*_grad_disp_neighbor[2])[_qp]));

  _F_avg[_qp] = RankTwoTensor::Identity() +
                0.5 * (F + F_neighbor - 2. * RankTwoTensor::Identity());

  _L_avg[_qp] =
      RankTwoTensor::Identity() - _F_avg_old[_qp] * _F_avg[_qp].inverse();
}

void CZMMaterialLD::PolarDecomposition(const RankTwoTensor &F, RankTwoTensor &R,
                                       RankTwoTensor &U) const {

  // copy paste from tnesor mechanics
  std::vector<Real> e_value(3);
  RankTwoTensor e_vector, N1, N2, N3;

  RankTwoTensor C = F.transpose() * F;
  C.symmetricEigenvaluesEigenvectors(e_value, e_vector);

  const Real lambda1 = std::sqrt(e_value[0]);
  const Real lambda2 = std::sqrt(e_value[1]);
  const Real lambda3 = std::sqrt(e_value[2]);

  N1.vectorOuterProduct(e_vector.column(0), e_vector.column(0));
  N2.vectorOuterProduct(e_vector.column(1), e_vector.column(1));
  N3.vectorOuterProduct(e_vector.column(2), e_vector.column(2));

  U = N1 * lambda1 + N2 * lambda2 + N3 * lambda3;
  R = F * U.inverse();
}

void CZMMaterialLD::computeRdot(const RankTwoTensor &F, const RankTwoTensor &L,
                                const RankTwoTensor &R, const RankTwoTensor &U,
                                RankTwoTensor &Rdot) {

  RankTwoTensor Fdot = L * F;
  RankTwoTensor Uhat = U.trace() * RankTwoTensor::Identity() - U;
  Rdot = 1. / Uhat.det() * (R * Uhat) *
         (R.transpose() * Fdot - Fdot.transpose() * R) * Uhat.transpose();
}

void CZMMaterialLD::computeSdot(const RankTwoTensor &L,
                                const RealVectorValue &n, Real &sdot) {
  Real t1 = n * (L * n);
  sdot = L.trace() - t1;
}

void CZMMaterialLD::computeTraction() {
  _traction_dot[_qp] = _K * _displacement_jump_dot[_qp];
  _traction[_qp] = _traction_old[_qp] + _traction_dot[_qp];
}
