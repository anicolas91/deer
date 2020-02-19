//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Assembly.h"
#include "CZMStrainRate.h"
#include "DenseMatrix.h"
#include "RotationMatrix.h"

registerMooseObject("DeerApp", CZMStrainRate);

InputParameters CZMStrainRate::validParams() {
  InputParameters params = InterfaceMaterial::validParams();

  params.addClassDescription("Base class for cohesive zone mateirla models");
  params.addRequiredCoupledVar(
      "displacements",
      "The string of displacements suitable for the problem statement");
  params.set<bool>("use_displaced_mesh") = false;

  return params;
}

CZMStrainRate::CZMStrainRate(const InputParameters &parameters)
    : InterfaceMaterial(parameters), _normals(_assembly.normals()),
      _ndisp(coupledComponents("displacements")), _disp(_ndisp),
      _disp_neighbor(_ndisp),

      _interface_strain(declareProperty<RankTwoTensor>("interface_strain")),
      _interface_opening_strain(
          declareProperty<RankTwoTensor>("interface_opening_strain")),
      _interface_sliding_strain(
          declareProperty<RankTwoTensor>("interface_sliding_strain")),
      _interface_strain_rate(
          declareProperty<RankTwoTensor>("interface_strain_rate")),
      _interface_opening_strain_rate(
          declareProperty<RankTwoTensor>("interface_opening_strain_rate")),
      _interface_sliding_strain_rate(
          declareProperty<RankTwoTensor>("interface_sliding_strain_rate")) {

  if (_ndisp > 3 || _ndisp < 1)
    mooseError("the CZM material requires 1, 2 or 3 displacement variables");

  // intializing displacement vectors
  for (unsigned int i = 0; i < _ndisp; ++i) {
    _disp[i] = &coupledValue("displacements", i);
    _disp_neighbor[i] = &coupledNeighborValue("displacements", i);
  }
}

void CZMStrainRate::computeQpProperties() {
  RealVectorValue _displacement_jump_global;
  RealVectorValue _displacement_jump_dot_global;
  // computing the actual displacement jump
  for (unsigned int i = 0; i < _ndisp; i++) {
    _displacement_jump_global(i) = (*_disp_neighbor[i])[_qp] - (*_disp[i])[_qp];
    _displacement_jump_dot_global(i) = _displacement_jump_global(i) / _dt;
  }
  for (unsigned int i = _ndisp; i < 3; i++) {
    _displacement_jump_global(i) = 0;
    _displacement_jump_dot_global(i) = 0;
  }

  Real opening_velocity = _displacement_jump_dot_global * _normals[_qp];
  Real opening = _displacement_jump_global * _normals[_qp];

  _interface_strain_rate[_qp] =
      ((outer_product(_normals[_qp], _displacement_jump_dot_global)) +
       (outer_product(_displacement_jump_dot_global, _normals[_qp]))) *
      .5;

  _interface_opening_strain_rate[_qp] =
      outer_product(_normals[_qp], _normals[_qp]) * opening_velocity;

  _interface_sliding_strain_rate[_qp] =
      _interface_strain_rate[_qp] - _interface_opening_strain_rate[_qp];

  _interface_strain[_qp] =
      (outer_product(_normals[_qp], _displacement_jump_global) +
       outer_product(_displacement_jump_global, _normals[_qp])) *
      .5;

  _interface_opening_strain[_qp] =
      outer_product(_normals[_qp], _normals[_qp]) * opening;
  _interface_sliding_strain[_qp] =
      _interface_strain[_qp] - _interface_opening_strain[_qp];
}
