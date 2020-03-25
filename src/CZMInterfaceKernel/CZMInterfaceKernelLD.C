//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CZMInterfaceKernelLD.h"

registerMooseObject("TensorMechanicsApp", CZMInterfaceKernelLD);

InputParameters CZMInterfaceKernelLD::validParams() {
  InputParameters params = InterfaceKernelDisconnectedMesh::validParams();
  params.addRequiredParam<unsigned int>(
      "component", "the component of the "
                   "displacement vector this kernel is working on:"
                   " component == 0, ==> X"
                   " component == 1, ==> Y"
                   " component == 2, ==> Z");

  params.addRequiredCoupledVar("displacements",
                               "the string containing displacement variables");

  params.addClassDescription(
      "Interface kernel for use with cohesive zone models (CZMs) that "
      "compute traction as a function of the displacement jump");

  return params;
}

CZMInterfaceKernelLD::CZMInterfaceKernelLD(const InputParameters &parameters)
    : InterfaceKernelDisconnectedMesh(parameters),
      _component(getParam<unsigned int>("component")),
      _ndisp(coupledComponents("displacements")), _disp_var(_ndisp),
      _disp_neighbor_var(_ndisp),
      _traction_global(
          getMaterialPropertyByName<RealVectorValue>("traction_global")),
      _traction_derivatives_global(getMaterialPropertyByName<RankTwoTensor>(
          "traction_derivatives_global")),
      _traction_global_neighbor(getMaterialPropertyByName<RealVectorValue>(
          "traction_global_neighbor")),
      _traction_derivatives_global_neighbor(
          getMaterialPropertyByName<RankTwoTensor>(
              "traction_derivatives_global_neighbor")) {
  // std::cout << "use_displaced_mesh: " << _use_displaced_mesh << std::endl;
  // if (getParam<bool>("use_displaced_mesh") == false)
  //   mooseError(
  //       "CZMInterfaceKernelLD cannot be used with use_displaced_mesh =
  //       false");

  for (unsigned int i = 0; i < _ndisp; ++i) {
    _disp_var[i] = coupled("displacements", i);
    _disp_neighbor_var[i] = coupled("displacements", i);
  }
}

Real CZMInterfaceKernelLD::computeQpResidual(Moose::DGResidualType type) {

  Real r = 0;
  switch (type) {
  // [test_slave-test_master]*T where T represents the traction.
  case Moose::Element:
    r = -_traction_global[_qp](_component) * _test[_i][_qp];
    // std::cout << "master: qp " << _qp;
    break;
  case Moose::Neighbor:
    r = _traction_global_neighbor[_qp](_component) * _test_neighbor[_i][_qp];
    // std::cout << "neighbor: qp " << _qp;
    break;
  }
  // std::cout << " _component " << _component << " traction " << r <<
  // std::endl;
  return r;
}

Real CZMInterfaceKernelLD::computeQpJacobian(Moose::DGJacobianType type) {
  // retrieve the diagonal Jacobian coefficient dependning on the  displacement
  // component (_component) this kernel is working on
  Real jac = 0;

  switch (type) {
  case Moose::ElementElement: // Residual_sign -1  ddeltaU_ddisp sign -1;
    jac = _traction_derivatives_global[_qp](_component, _component) *
          _test[_i][_qp] * _phi[_j][_qp];
    break;
  case Moose::ElementNeighbor: // Residual_sign -1  ddeltaU_ddisp sign 1;
    jac = -_traction_derivatives_global[_qp](_component, _component) *
          _test[_i][_qp] * _phi_neighbor[_j][_qp];
    break;
  case Moose::NeighborElement: // Residual_sign 1  ddeltaU_ddisp sign -1;
    jac = -_traction_derivatives_global_neighbor[_qp](_component, _component) *
          _test_neighbor[_i][_qp] * _phi[_j][_qp];
    break;
  case Moose::NeighborNeighbor: // Residual_sign 1  ddeltaU_ddisp sign 1;
    jac = _traction_derivatives_global_neighbor[_qp](_component, _component) *
          _test_neighbor[_i][_qp] * _phi_neighbor[_j][_qp];
    break;
  }
  return jac;
}

Real CZMInterfaceKernelLD::computeQpOffDiagJacobian(Moose::DGJacobianType type,
                                                    unsigned int jvar) {

  // find the displacement component associated to jvar
  unsigned int off_diag_component;
  for (off_diag_component = 0; off_diag_component < _ndisp;
       off_diag_component++)
    if (_disp_var[off_diag_component] == jvar)
      break;

  mooseAssert(
      off_diag_component < _ndisp,
      "CZMInterfaceKernelLD::computeQpOffDiagJacobian wrong offdiagonal "
      "variable");

  Real jac = 0;

  switch (type) {
  case Moose::ElementElement: // Residual_sign -1  ddeltaU_ddisp sign -1;
    jac = _traction_derivatives_global[_qp](_component, off_diag_component) *
          _test[_i][_qp] * _phi[_j][_qp];
    break;
  case Moose::ElementNeighbor: // Residual_sign -1  ddeltaU_ddisp sign 1;
    jac = -_traction_derivatives_global[_qp](_component, off_diag_component) *
          _test[_i][_qp] * _phi_neighbor[_j][_qp];
    break;
  case Moose::NeighborElement: // Residual_sign 1  ddeltaU_ddisp sign -1;
    jac = -_traction_derivatives_global_neighbor[_qp](_component,
                                                      off_diag_component) *
          _test_neighbor[_i][_qp] * _phi[_j][_qp];
    break;
  case Moose::NeighborNeighbor: // Residual_sign 1  ddeltaU_ddisp sign 1;
    jac = _traction_derivatives_global_neighbor[_qp](_component,
                                                     off_diag_component) *
          _test_neighbor[_i][_qp] * _phi_neighbor[_j][_qp];
    break;
  }
  return jac;
}
