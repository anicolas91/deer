//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "INSStressStrain.h"
#include "Function.h"
#include "Assembly.h"
#include "NonlinearSystemBase.h"
// #include "INSADObjectTracker.h"

registerMooseObject("NavierStokesApp", INSStressStrain);

InputParameters
INSStressStrain::validParams()
{
  InputParameters params = Material::validParams();
  params.addRequiredCoupledVar("velocity", "The velocity");
  params.addCoupledVar("pressure", 0, "The pressure");
  params.addParam<MaterialPropertyName>("mu_name", "mu", "The name of the dynamic viscosity");
  return params;
}

INSStressStrain::INSStressStrain(const InputParameters & parameters)
  : Material(parameters),
    // velocities and gradients
    _velocity(coupledVectorValue("velocity")),
    _grad_velocity(coupledVectorGradient("velocity")),
    // Stresses, strain rates, and pressure
    _stress(declareProperty<RankTwoTensor>("stress")),
    _strain_rate(declareProperty<RankTwoTensor>("strain_rate")),
    _pressure(coupledValue("pressure")),
    // constants and viscosity
    _mu(getMaterialProperty<Real>("mu_name")) //converted from AD
    // _coord_sys(_assembly.coordSystem())
{
}

void
INSStressStrain::computeQpProperties()
{
  _stress[_qp].zero();
  _stress[_qp](0,0) = -_pressure[_qp] + 2.0 * _mu[_qp] * _grad_velocity[_qp](0,0);
  _stress[_qp](1,1) = -_pressure[_qp] + 2.0 * _mu[_qp] * _grad_velocity[_qp](1,1);
  _stress[_qp](2,2) = -_pressure[_qp] + 2.0 * _mu[_qp] * _grad_velocity[_qp](2,2);
  _stress[_qp](0,1) =  _mu[_qp] * (_grad_velocity[_qp](0,1)+_grad_velocity[_qp](1,0));
  _stress[_qp](1,0) =  _stress[_qp](0,1);
  _stress[_qp](0,2) =  _mu[_qp] * (_grad_velocity[_qp](0,2)+_grad_velocity[_qp](2,0));
  _stress[_qp](2,0) =  _stress[_qp](0,2);
  _stress[_qp](1,2) =  _mu[_qp] * (_grad_velocity[_qp](1,2)+_grad_velocity[_qp](2,1));
  _stress[_qp](2,1) =  _stress[_qp](1,2);

  _strain_rate[_qp].zero();
  _strain_rate[_qp](0,0) =  1.0*_grad_velocity[_qp](0,0);
  _strain_rate[_qp](1,1) =  1.0*_grad_velocity[_qp](1,1);
  _strain_rate[_qp](2,2) =  1.0*_grad_velocity[_qp](2,2);
  _strain_rate[_qp](0,1) = 1.0*(_grad_velocity[_qp](0,1)+_grad_velocity[_qp](1,0)) /2.0;
  _strain_rate[_qp](1,0) = _strain_rate[_qp](0,1);
  _strain_rate[_qp](0,2) = 1.0*(_grad_velocity[_qp](0,2)+_grad_velocity[_qp](2,0)) /2.0;
  _strain_rate[_qp](2,0) = _strain_rate[_qp](0,2);
  _strain_rate[_qp](1,2) = 1.0*(_grad_velocity[_qp](1,2)+_grad_velocity[_qp](2,1)) /2.0;
  _strain_rate[_qp](2,1) = _strain_rate[_qp](1,2);

  // Moose::out<<"Stress and strains calculated"<<"\n";


  // if (_coord_sys == Moose::COORD_RZ)
  // {
  //   const Real r = _q_point[_qp](0);
  //
  //   _adstress[_qp](2,2) = -_pressure[_qp] + 2.0 * _mu[_qp] * (_velocity[_qp](0) + _grad_velocity[_qp](2,2))/r;
  //   _adstress[_qp](1,2) =  _mu[_qp] * (_grad_velocity[_qp](1,2)/r + _grad_velocity[_qp](2,1));
  //   _adstress[_qp](2,1) =  _adstress[_qp](1,2);
  //   _adstress[_qp](0,2) =  _mu[_qp] * ( r * _grad_velocity[_qp](2,0)/r + _grad_velocity[_qp](0,2)/ r ); //needs fixing
  //
  //   _adstrain_rate[_qp](2,2) = (_velocity[_qp](0) + _grad_velocity[_qp](2,2))/r;
  //   _adstrain_rate[_qp](1,2) = (_grad_velocity[_qp](1,2)/r + _grad_velocity[_qp](2,1))/2.0;
  //   _adstrain_rate[_qp](2,1) =  _adstrain_rate[_qp](1,2);
  //   _adstrain_rate[_qp](0,2) = ( r * _grad_velocity[_qp](2,0)/r + _grad_velocity[_qp](0,2)/ r ) /2.0; //needs fixing
  //
  // }

// Moose::out<<"Stress : "<<_stress[_qp]<<"\n";
// Moose::out<<"Stress 0,0: "<<_stress[_qp](0,0)<<"\n";
// Moose::out<<"srate 0,0: "<<_strain_rate[_qp](0,0)<<"\n";

// Moose::out<<"Stress 1,1: "<<_stress[_qp](1,1)<<"\n";
// Moose::out<<"Stress 2,2: "<<_stress[_qp](2,2)<<"\n";
// Moose::out<<"Stress 2,1: "<<_stress[_qp](2,1)<<"\n";
// Moose::out<<"Stress 1,2: "<<_stress[_qp](1,2)<<"\n";
// Moose::out<<"Stress 0,2: "<<_stress[_qp](0,2)<<"\n";
// Moose::out<<"Stress 1,2: "<<_stress[_qp](1,2)<<"\n";

// Moose::out<<"only rr shear: "<<shear<<"\n";

  	// _mu[_qp] = mu_in;


}
