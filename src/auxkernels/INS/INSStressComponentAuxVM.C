//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "INSStressComponentAuxVM.h"
#include "MooseMesh.h"

registerMooseObject("NavierStokesApp", INSStressComponentAuxVM);

InputParameters
INSStressComponentAuxVM::validParams()
{
  InputParameters params = AuxKernel::validParams();

  params.addClassDescription("This class computes the Von Mises stress based on 2d radial");
  params.addCoupledVar("velocity_r", "The velocity component on x (r = radial)");
  params.addCoupledVar("velocity_z", "The velocity component on y (z = axial)");
  params.addCoupledVar("pressure", 0, "The pressure");
  params.addParam<MaterialPropertyName>("mu_name", "mu", "The viscosity");

  return params;
}

INSStressComponentAuxVM::INSStressComponentAuxVM(const InputParameters & parameters)
  : AuxKernel(parameters),
    _velocity_r(coupledValue("velocity_r")),
    _velocity_z(coupledValue("velocity_z")),
    _grad_velocity_r(isCoupled("velocity_r") ? coupledGradient("velocity_r") : _grad_zero),
    _grad_velocity_z(isCoupled("velocity_z") ? coupledGradient("velocity_z") : _grad_zero),
    _pressure(coupledValue("pressure")),
    _mu(getMaterialProperty<Real>("mu_name")) //converted from AD
{
}

Real
INSStressComponentAuxVM::computeValue()
{
  const Real r = _q_point[_qp](0);
  auto && srr = -_pressure[_qp] + 2.0 *_mu[_qp] * _grad_velocity_r[_qp](0);
  auto && szz = -_pressure[_qp] + 2.0 *_mu[_qp] * _grad_velocity_z[_qp](1);
  auto && soo = -_pressure[_qp] + 2.0 *_mu[_qp] * _velocity_r[_qp] / r;
  auto && srz = +_mu[_qp] * (_grad_velocity_r[_qp](1)+_grad_velocity_z[_qp](0));

  auto && svm = std::sqrt(   0.5*( (srr-szz)*(srr-szz)
                                  +(szz-soo)*(szz-soo)
                                  +(soo-srr)*(soo-srr) )
                           + 3.0*srz*srz);

return svm;
}
