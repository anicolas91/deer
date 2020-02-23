//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html
typedef unsigned int uint;
#include "CZMMaterialBaseStateful.h"
#include "RotationMatrix.h"

InputParameters CZMMaterialBaseStateful::validParams() {
  InputParameters params = CZMMaterialBase::validParams();
  params.addClassDescription("this material class is used when defining a "
                             "stateful viscous cohesive zone model");
  return params;
}

CZMMaterialBaseStateful::CZMMaterialBaseStateful(
    const InputParameters &parameters)
    : CZMMaterialBase(parameters), _disp_dot(_ndisp),
      _disp_neighbor_dot(_ndisp),
      _displacement_jump_global_old(
          getMaterialPropertyOld<RealVectorValue>("displacement_jump_global")),
      _displacement_jump_old(
          getMaterialPropertyOld<RealVectorValue>("displacement_jump")),

      _traction_global_old(
          getMaterialPropertyOld<RealVectorValue>("traction_global")),
      _traction_old(getMaterialPropertyOld<RealVectorValue>("traction")),

      _displacement_jump_dot_global(
          declareProperty<RealVectorValue>("displacement_jump_dot_global")),
      _displacement_jump_dot(
          declareProperty<RealVectorValue>("displacement_jump_dot")),

      _displacement_jump_dot_global_old(getMaterialPropertyOld<RealVectorValue>(
          "displacement_jump_dot_global")),
      _displacement_jump_dot_old(
          getMaterialPropertyOld<RealVectorValue>("displacement_jump_dot")) {
  // initializing the displacement vectors
  for (unsigned int i = 0; i < _ndisp; ++i) {
    _disp_dot[i] = &coupledDot("displacements", i);
    _disp_neighbor_dot[i] = &coupledNeighborValueDot("displacements", i);
  }
}

void CZMMaterialBaseStateful::computeQpProperties() {

  RealTensorValue RotationGlobalToLocal =
      RotationMatrix::rotVec1ToVec2(_normals[_qp], RealVectorValue(1, 0, 0));

  // computing the displacement jump
  for (unsigned int i = 0; i < _ndisp; i++) {
    _displacement_jump_global[_qp](i) =
        (*_disp_neighbor[i])[_qp] - (*_disp[i])[_qp];
    _displacement_jump_dot_global[_qp](i) =
        (*_disp_neighbor_dot[i])[_qp] - (*_disp_dot[i])[_qp];
  }
  for (unsigned int i = _ndisp; i < 3; i++) {
    _displacement_jump_global[_qp](i) = 0;
    _displacement_jump_dot_global[_qp](i) = 0;
  }

  // rotate the displacement jump to the local coordiante system
  _displacement_jump[_qp] =
      RotationGlobalToLocal * _displacement_jump_global[_qp];
  _displacement_jump_dot[_qp] =
      RotationGlobalToLocal * _displacement_jump_dot_global[_qp];

  // compute local traction
  _traction[_qp] = computeTraction();

  // compute local traction derivatives wrt the displacement jump
  _traction_derivatives[_qp] = computeTractionDerivatives();
  computeTractionDerivativesAndNewMaterialPropertiesLocal();

  // rotate local traction and derivatives to the global coordinate system
  _traction_global[_qp] = RotationGlobalToLocal.transpose() * _traction[_qp];
  _traction_derivatives_global[_qp] = _traction_derivatives[_qp];
  _traction_derivatives_global[_qp].rotate(RotationGlobalToLocal.transpose());
}

void CZMMaterialBaseStateful::initQpStatefulProperties() {

  // intialize displacement jump properties
  for (unsigned int i = 0; i < 3; i++) {
    _traction[_qp](i) = 0;
    _traction_global[_qp](i) = 0;
    _displacement_jump_global[_qp](i) = 0;
    _displacement_jump[_qp](i) = 0;
    _displacement_jump_dot_global[_qp](i) = 0;
    _displacement_jump_dot[_qp](i) = 0;
  }
}
