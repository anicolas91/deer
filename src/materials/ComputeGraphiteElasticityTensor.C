//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ComputeGraphiteElasticityTensor.h"
#include "ComputeElasticityTensor.h"
#include "RotationTensor.h"

registerMooseObject("DeerApp", ComputeGraphiteElasticityTensor);

template <>
InputParameters
validParams<ComputeGraphiteElasticityTensor>()
{
  InputParameters params = validParams<ComputeRotatedElasticityTensorBase>();
  params.addClassDescription("Compute an elasticity tensor with changes.");
  params.addRequiredParam<MaterialPropertyName>("prefactor_par", "Name of material defining the variable dependence");
  params.addRequiredParam<MaterialPropertyName>("prefactor_per", "Name of material defining the variable dependence");
  params.addRequiredParam<Real>("Epar", "Youngs modulus parallel to the extrusion");
  params.addRequiredParam<Real>("Eper", "Youngs modulus perpendicular to the extrusion");
  params.addRequiredParam<Real>("nu", "poissons ratio");
  params.addRequiredParam<std::vector<Real>>("C_ijkl", "Stiffness tensor for material");
  params.addParam<MooseEnum>(
      "fill_method", RankFourTensor::fillMethodEnum() = "symmetric9", "The fill method");

  return params;
}

ComputeGraphiteElasticityTensor::ComputeGraphiteElasticityTensor(const InputParameters & parameters)
  : ComputeRotatedElasticityTensorBase(parameters),
    _prefactor_par(getMaterialProperty<Real>("prefactor_par")),
    _prefactor_per(getMaterialProperty<Real>("prefactor_per")),
    _Epar(getParam<Real>("Epar")),
    _Eper(getParam<Real>("Eper")),
    _nu(getParam<Real>("nu")),
    _Cijkl(getParam<std::vector<Real>>("C_ijkl"),
           (RankFourTensor::FillMethod)(int)getParam<MooseEnum>("fill_method"))
{
  if (!isParamValid("elasticity_tensor_prefactor"))
    issueGuarantee(_elasticity_tensor_name, Guarantee::CONSTANT_IN_TIME);

  if (_Cijkl.isIsotropic())
    issueGuarantee(_elasticity_tensor_name, Guarantee::ISOTROPIC);
  else
  {

    // modify the tensor to make it anisotripic and dependent on properties



    // Define a rotation according to Euler angle parameters
    RotationTensor R(_Euler_angles); // R type: RealTensorValue

    // rotate elasticity tensor
    _Cijkl.rotate(R);
  }
}

void
ComputeGraphiteElasticityTensor::computeQpElasticityTensor()
{
  // Assign elasticity tensor at a given quad point
  _elasticity_tensor[_qp] = _Cijkl;

  // some lil helper
  double nnn = 1/(-2*pow(_nu,2)-_nu+1);

  // redefine portions of the tensor -- C11 C12 C13 C21 C22 C23 C31 C32 C33
  _elasticity_tensor[_qp](0,0,0,0) = -_prefactor_par[_qp]*_Epar*(_nu-1)*nnn;
  _elasticity_tensor[_qp](1,1,1,1) = -_prefactor_par[_qp]*_Epar*(_nu-1)*nnn;
  _elasticity_tensor[_qp](2,2,2,2) = -_prefactor_per[_qp]*_Eper*(_nu-1)*nnn;
  _elasticity_tensor[_qp](0,0,1,1) = _prefactor_par[_qp]*_Epar*(_nu)*nnn;
  _elasticity_tensor[_qp](0,0,2,2) = _prefactor_par[_qp]*_Epar*(_nu)*nnn;
  _elasticity_tensor[_qp](1,1,0,0) = _prefactor_par[_qp]*_Epar*(_nu)*nnn;
  _elasticity_tensor[_qp](1,1,2,2) = _prefactor_par[_qp]*_Epar*(_nu)*nnn;
  _elasticity_tensor[_qp](2,2,0,0) = _prefactor_per[_qp]*_Eper*(_nu)*nnn;
  _elasticity_tensor[_qp](2,2,1,1) = _prefactor_per[_qp]*_Eper*(_nu)*nnn;

  // redefine portions of the tensor -- C44 C55 C66
  // C44
  _elasticity_tensor[_qp](1,2,1,2) = 1000*(_prefactor_par[_qp] + _prefactor_per[_qp]);
  _elasticity_tensor[_qp](1,2,2,1) = 1000*(_prefactor_par[_qp] + _prefactor_per[_qp]);
  _elasticity_tensor[_qp](2,1,2,1) = 1000*(_prefactor_par[_qp] + _prefactor_per[_qp]);
  _elasticity_tensor[_qp](2,1,1,2) = 1000*(_prefactor_par[_qp] + _prefactor_per[_qp]);

  // C55
  _elasticity_tensor[_qp](2,0,2,0) = 1000*(_prefactor_par[_qp] + _prefactor_per[_qp]);
  _elasticity_tensor[_qp](2,0,0,2) = 1000*(_prefactor_par[_qp] + _prefactor_per[_qp]);
  _elasticity_tensor[_qp](0,2,0,2) = 1000*(_prefactor_par[_qp] + _prefactor_per[_qp]);
  _elasticity_tensor[_qp](0,2,2,0) = 1000*(_prefactor_par[_qp] + _prefactor_per[_qp]);

  // C66
  _elasticity_tensor[_qp](0,1,0,1) = 1000*(_prefactor_par[_qp] + _prefactor_per[_qp]);
  _elasticity_tensor[_qp](0,1,1,0) = 1000*(_prefactor_par[_qp] + _prefactor_per[_qp]);
  _elasticity_tensor[_qp](1,0,1,0) = 1000*(_prefactor_par[_qp] + _prefactor_per[_qp]);
  _elasticity_tensor[_qp](1,0,0,1) = 1000*(_prefactor_par[_qp] + _prefactor_per[_qp]);


//  _elasticity_tensor[_qp](1,2,1,2);
}
























// #include "ComputeGraphiteElasticityTensor.h"
//
// #include "Function.h"
// #include "RankTwoTensor.h"
//
// registerMooseObject("DeerApp", ComputeGraphiteElasticityTensor);
//
// template <>
// InputParameters
// validParams<ComputeGraphiteElasticityTensor>()
// {
//   InputParameters params = validParams<ComputeElasticityTensorBase>();
//   params.addClassDescription("Compute Graphite Elasticity Tensor from the compliance tensor");
//   params.addRequiredParam<FunctionName>("E_E0par", "E/E0 par");
//   params.addRequiredParam<FunctionName>("E_E0per", "E/E0 per");
//   params.addRequiredParam<Real>("Epar", 9000, "Youngs modulus parallel to the extrusion");
//   params.addRequiredParam<Real>("Eper", 9000, "Youngs modulus perpendicular to the extrusion");
//   params.addRequiredParam<Real>("nu", 0.15, "poissons ratio");
//   params.addRequiredCoupledVar("rtemp", "rel temperature");
//   params.addRequiredCoupledVar("rdose", "rel fluence");
//   return params;
// }
//
// ComputeGraphiteElasticityTensor::ComputeGraphiteElasticityTensor(const InputParameters & parameters)
//   : ComputeElasticityTensorBase(parameters),
//   _Cijkl(getParam<std::vector<Real>>("C_ijkl"),(RankFourTensor::"symmetric9"))
//     _E_E0par(getFunction("E_E0par")),
//     _E_E0per(getFunction("E_E0per")),
//     _Epar(getParam<Real>("Epar")),
//     _Eper(getParam<Real>("Eper")),
//     _mu(getParam<Real>("mu")),
//     _rtemp(coupledValue("rtemp")),
//     _rdose(coupledValue("rdose")),
// {
//
// }
//
// void ComputeGraphiteElasticityTensor::computeQpElasticityTensor()
// {
//   // Get changes in elasticity
//   double _eE0par = _E_E0par.value(_rtemp[_qp], rdose[_qp], _q_point[_qp]);
//   double _eE0per = _E_E0per.value(_rtemp[_qp], rdose[_qp], _q_point[_qp]);
//
//   // Assign elasticity tensor at a given quad point
//   _elasticity_tensor[_qp] = _Cijkl;
// }
