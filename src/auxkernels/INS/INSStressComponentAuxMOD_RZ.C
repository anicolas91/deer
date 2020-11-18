//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "INSStressComponentAuxMOD_RZ.h"
#include "MooseMesh.h"

registerMooseObject("NavierStokesApp", INSStressComponentAuxMOD_RZ);

InputParameters
INSStressComponentAuxMOD_RZ::validParams()
{
  InputParameters params = AuxKernel::validParams();

  params.addClassDescription("This class computes the stress component based on "
                             "pressure and velocity for incompressible Navier-Stokes");
  params.addCoupledVar("velocity", "The velocity component");
  params.addCoupledVar("pressure", 0, "The pressure");
  params.addRangeCheckedParam<unsigned int>("comp", 0, "0<=comp<=2", "The component");
  params.addParam<MaterialPropertyName>("mu_name", "mu", "The viscosity");

  return params;
}

INSStressComponentAuxMOD_RZ::INSStressComponentAuxMOD_RZ(const InputParameters & parameters)
  : AuxKernel(parameters),
    _grad_velocity(isCoupled("velocity") ? coupledGradient("velocity") : _grad_zero),
    _velocity(coupledValue("velocity")),
    _pressure(coupledValue("pressure")),
    _comp(getParam<unsigned>("comp")),
    _mu(getMaterialProperty<Real>("mu_name")) //converted from AD
{
}

Real
INSStressComponentAuxMOD_RZ::computeValue()
{
  auto && s = -_pressure[_qp] + 2.0 *_mu[_qp] * _grad_velocity[_qp](_comp);

  if (_comp == 2)
  {
    const Real r = _q_point[_qp](0);
    s = -_pressure[_qp] + 2.0 * _mu[_qp] * _velocity[_qp] / r;
  }

return s;
}
