//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GeneralizedKelvinVoigtModelGraphite.h"
#include "RankFourTensor.h"

registerMooseObject("DeerApp", GeneralizedKelvinVoigtModelGraphite);

template <>
InputParameters
validParams<GeneralizedKelvinVoigtModelGraphite>()
{
  InputParameters params = validParams<GeneralizedKelvinVoigtBase>();
  params.addClassDescription(
      "Generalized Kelvin-Voigt model composed of a serial assembly of unit Kelvin-Voigt modules");
  // params.addParam<std::string>("base_name", "Material property base name");
  params.addRequiredParam<Real>("alpha", "Alpha - primary creep constant");
  params.addRequiredParam<Real>("gamma", "Gamma - primary creep constant");
  params.addRequiredParam<Real>("kappa", "Kappa - secondary creep constant");
  params.addRequiredParam<Real>("young_modulus_par", "initial elastic modulus parallel");
  params.addRequiredParam<Real>("young_modulus_per", "initial elastic modulus perpendicular");
  params.addRequiredParam<FunctionName>("fluence_function_dT","derivative of fluence");
  params.addRequiredParam<MaterialPropertyName>("EoverE0_par","Rate of elastic stiffness parallel");
  params.addRequiredParam<MaterialPropertyName>("EoverE0_per","Rate of elastic stiffness perpendicular");
  params.addRequiredParam<Real>("poisson_ratio", "initial poisson ratio of the material");
  // params.addRequiredParam<std::vector<Real>>(
      // "creep_modulus", "list of the elastic moduli of the different springs in the material");
  // params.addRequiredParam<std::vector<Real>>(
  //     "creep_viscosity",
  //     "list of the characteristic times of the different dashpots in the material");
  params.addParam<std::vector<Real>>(
      "creep_ratio", "list of the poisson ratios of the different springs in the material");
  params.set<bool>("force_recompute_properties") = false;
  params.suppressParameter<bool>("force_recompute_properties");
  return params;
}

GeneralizedKelvinVoigtModelGraphite::GeneralizedKelvinVoigtModelGraphite(const InputParameters & parameters)
  : GeneralizedKelvinVoigtBase(parameters),
    // _base_name(isParamValid("base_name") ? getParam<std::string>("base_name") + "_" : ""),
    // _elasticity_tensor(getMaterialPropertyByName<RankFourTensor>(_base_name + "elasticity_tensor")),
    // _Ci(getParam<std::vector<Real>>("creep_modulus").size()),
    _Ci(1),
    // _eta_i(getParam<std::vector<Real>>("creep_viscosity")),
    _eta_i(2),
    // _Si(getParam<std::vector<Real>>("creep_modulus").size()),
    _Si(1),
    _fluence_function_dT(getFunction("fluence_function_dT")),
    _alpha(getParam<Real>("alpha")),
    _young_modulus_par(getParam<Real>("young_modulus_par")),
    _young_modulus_per(getParam<Real>("young_modulus_per")),
    _poisson_ratio(getParam<Real>("poisson_ratio")),
    _EoverE0_par(getMaterialProperty<Real>("EoverE0_par")),
    _EoverE0_per(getMaterialProperty<Real>("EoverE0_per"))

{
  // Real alpha             = getParam<Real>("alpha");
  Real gamma             = getParam<Real>("gamma");
  Real kappa             = getParam<Real>("kappa");
  // Real young_modulus_par = getParam<Real>("young_modulus_par");
  // Real young_modulus_per = getParam<Real>("young_modulus_per");
  // Real poisson_ratio     = getParam<Real>("poisson_ratio");

  // _C0.fillFromInputVector({_young_modulus_par, _poisson_ratio}, RankFourTensor::symmetric_isotropic_E_nu);
  // _S0 = _C0.invSymm();

  // std::vector<Real> creep_modulus = getParam<std::vector<Real>>("creep_modulus");
  std::vector<Real> creep_modulus;
  creep_modulus.resize(_Ci.size(), _young_modulus_par);

  std::vector<Real> creep_ratio;
  if (isParamValid("creep_ratio"))
    creep_ratio = getParam<std::vector<Real>>("creep_ratio");
  else
    creep_ratio.resize(_Ci.size(), _poisson_ratio);

  if (creep_modulus.size() != _Ci.size())
    mooseError("incompatible number of creep moduli and viscosities");
  if (creep_ratio.size() != _Ci.size())
    mooseError("incompatible number of creep ratios and viscosities");
  if (!(_Ci.size() == _eta_i.size() || _Ci.size() + 1 == _eta_i.size()))
    mooseError("incompatible number of creep ratios and viscosities");



  /// calculate the spring STIFFNESS IN GKV
  for (unsigned int i = 0; i < _Ci.size(); ++i)
  {
    _Ci[i].fillFromInputVector({creep_modulus[i], creep_ratio[i]},
                               RankFourTensor::symmetric_isotropic_E_nu);
    // if (i == 0)
    //   _Ci[i] = _Ci[i]/alpha;

    _Si[i] = _Ci[i].invSymm();
  }

  /// calculate the damper VISCOSITIES IN GKV
  for (unsigned int i = 0; i < _eta_i.size(); ++i)
  {
    if (i == 0)   // viscosity for Primary creep
      _eta_i[i] = gamma;
    if (i == 1)   // viscosity for secondary creep
      _eta_i[i] = _young_modulus_par/kappa;

    if (_eta_i[i] < 0 || MooseUtils::absoluteFuzzyEqual(_eta_i[i], 0.0))
      mooseError("material viscosity must be strictly > 0");
  }

  _components = _eta_i.size();
  _has_longterm_dashpot = (_eta_i.size() == _Ci.size() + 1);

  issueGuarantee(_elasticity_tensor_name, Guarantee::ISOTROPIC);
  declareViscoelasticProperties();
}

void
GeneralizedKelvinVoigtModelGraphite::computeQpViscoelasticProperties()
{
  double nnn = 1/(-2*pow(_poisson_ratio,2)-_poisson_ratio+1);
  // _first_elasticity_tensor[_qp] = _C0;
  // redefine portions of the tensor -- C11 C12 C13 C21 C22 C23 C31 C32 C33
  // C11 and C22
    _first_elasticity_tensor[_qp](0,0,0,0) = _first_elasticity_tensor[_qp](1,1,1,1)
  =-_EoverE0_par[_qp]*_young_modulus_par*(_poisson_ratio-1)*nnn;
  // C33
    _first_elasticity_tensor[_qp](2,2,2,2)
  =-_EoverE0_per[_qp]*_young_modulus_per*(_poisson_ratio-1)*nnn;
  // C12 C13 C21 C23
    _first_elasticity_tensor[_qp](0,0,1,1) =  _first_elasticity_tensor[_qp](0,0,2,2)
  = _first_elasticity_tensor[_qp](1,1,0,0) =  _first_elasticity_tensor[_qp](1,1,2,2)
  = _EoverE0_par[_qp]*_young_modulus_par*(_poisson_ratio)*nnn;
  // C31 C32
    _first_elasticity_tensor[_qp](2,2,0,0) =  _first_elasticity_tensor[_qp](2,2,1,1)
  = _EoverE0_per[_qp]*_young_modulus_per*(_poisson_ratio)*nnn;
  // C44 C55 C66
    _first_elasticity_tensor[_qp](1,2,1,2) = _first_elasticity_tensor[_qp](1,2,2,1)
  = _first_elasticity_tensor[_qp](2,1,2,1) = _first_elasticity_tensor[_qp](2,1,1,2)
  = _first_elasticity_tensor[_qp](2,0,2,0) = _first_elasticity_tensor[_qp](2,0,0,2)
  = _first_elasticity_tensor[_qp](0,2,0,2) = _first_elasticity_tensor[_qp](0,2,2,0)
  = _first_elasticity_tensor[_qp](0,1,0,1) = _first_elasticity_tensor[_qp](0,1,1,0)
  = _first_elasticity_tensor[_qp](1,0,1,0) = _first_elasticity_tensor[_qp](1,0,0,1)
  = 1000*(_EoverE0_par[_qp]+_EoverE0_per[_qp]);
  // = 1*_young_modulus_par*nnn*(1-2*_poisson_ratio)/2;
  // _first_elasticity_tensor[_qp] = _C0;


  for (unsigned int i = 0; i < _Ci.size(); ++i)
  {
    (*_springs_elasticity_tensors[i])[_qp] = _Ci[i];
    if (i == 0)
      // C11 and C22
        (*_springs_elasticity_tensors[0])[_qp](0,0,0,0) = (*_springs_elasticity_tensors[0])[_qp](1,1,1,1)
      =-1*_young_modulus_par*(_poisson_ratio-1)*nnn/_alpha;
      // C33
        (*_springs_elasticity_tensors[0])[_qp](2,2,2,2)
      =-1*_young_modulus_per*(_poisson_ratio-1)*nnn/_alpha;
      // C12 C13 C21 C23
        (*_springs_elasticity_tensors[0])[_qp](0,0,1,1) =  (*_springs_elasticity_tensors[0])[_qp](0,0,2,2)
      = (*_springs_elasticity_tensors[0])[_qp](1,1,0,0) =  (*_springs_elasticity_tensors[0])[_qp](1,1,2,2)
      = 1*_young_modulus_par*(_poisson_ratio)*nnn/_alpha;
      // C31 C32
        (*_springs_elasticity_tensors[0])[_qp](2,2,0,0) =  (*_springs_elasticity_tensors[0])[_qp](2,2,1,1)
      = 1*_young_modulus_per*(_poisson_ratio)*nnn/_alpha;
      // C44 C55 C66
        (*_springs_elasticity_tensors[0])[_qp](1,2,1,2) = (*_springs_elasticity_tensors[0])[_qp](1,2,2,1)
      = (*_springs_elasticity_tensors[0])[_qp](2,1,2,1) = (*_springs_elasticity_tensors[0])[_qp](2,1,1,2)
      = (*_springs_elasticity_tensors[0])[_qp](2,0,2,0) = (*_springs_elasticity_tensors[0])[_qp](2,0,0,2)
      = (*_springs_elasticity_tensors[0])[_qp](0,2,0,2) = (*_springs_elasticity_tensors[0])[_qp](0,2,2,0)
      = (*_springs_elasticity_tensors[0])[_qp](0,1,0,1) = (*_springs_elasticity_tensors[0])[_qp](0,1,1,0)
      = (*_springs_elasticity_tensors[0])[_qp](1,0,1,0) = (*_springs_elasticity_tensors[0])[_qp](1,0,0,1)
      = 1000*(2)/_alpha;
  }

  for (unsigned int i = 0; i < _eta_i.size(); ++i)
    (*_dashpot_viscosities[i])[_qp] = _eta_i[i]/_fluence_function_dT.value(_t, _q_point[_qp]);
    // (*_dashpot_viscosities[i])[_qp] = _eta_i[i];
}

void
GeneralizedKelvinVoigtModelGraphite::computeQpViscoelasticPropertiesInv()
{
  // (*_first_elasticity_tensor_inv)[_qp] = _S0;
  (*_first_elasticity_tensor_inv)[_qp] = _first_elasticity_tensor[_qp].invSymm();

  for (unsigned int i = 0; i < _Si.size(); ++i)
  {
    (*_springs_elasticity_tensors_inv[i])[_qp] = _Si[i];
    if (i == 0)
    // (*_springs_elasticity_tensors_inv[0])[_qp] = (*_springs_elasticity_tensors[0])[_qp].invSymm();
      // C11 and C22
        (*_springs_elasticity_tensors_inv[0])[_qp](0,0,0,0) = (*_springs_elasticity_tensors_inv[0])[_qp](1,1,1,1)
      = 1/(1*_young_modulus_par)*_alpha;
      // C33
        (*_springs_elasticity_tensors_inv[0])[_qp](2,2,2,2)
      = 1/(1*_young_modulus_per)*_alpha;
      // C12 C13 C21 C23
        (*_springs_elasticity_tensors_inv[0])[_qp](0,0,1,1) =  (*_springs_elasticity_tensors_inv[0])[_qp](0,0,2,2)
      = (*_springs_elasticity_tensors_inv[0])[_qp](1,1,0,0) =  (*_springs_elasticity_tensors_inv[0])[_qp](1,1,2,2)
      = -_poisson_ratio/(1*_young_modulus_par)*_alpha;
      // C31 C32
        (*_springs_elasticity_tensors_inv[0])[_qp](2,2,0,0) =  (*_springs_elasticity_tensors_inv[0])[_qp](2,2,1,1)
      = -_poisson_ratio/(1*_young_modulus_per)*_alpha;
      // C44 C55 C66
        (*_springs_elasticity_tensors_inv[0])[_qp](1,2,1,2) = (*_springs_elasticity_tensors_inv[0])[_qp](1,2,2,1)
      = (*_springs_elasticity_tensors_inv[0])[_qp](2,1,2,1) = (*_springs_elasticity_tensors_inv[0])[_qp](2,1,1,2)
      = (*_springs_elasticity_tensors_inv[0])[_qp](2,0,2,0) = (*_springs_elasticity_tensors_inv[0])[_qp](2,0,0,2)
      = (*_springs_elasticity_tensors_inv[0])[_qp](0,2,0,2) = (*_springs_elasticity_tensors_inv[0])[_qp](0,2,2,0)
      = (*_springs_elasticity_tensors_inv[0])[_qp](0,1,0,1) = (*_springs_elasticity_tensors_inv[0])[_qp](0,1,1,0)
      = (*_springs_elasticity_tensors_inv[0])[_qp](1,0,1,0) = (*_springs_elasticity_tensors_inv[0])[_qp](1,0,0,1)
      = 1/((1+1)*1000)*_alpha;
  }
}
