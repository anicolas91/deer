//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "INSEshelbyTensor.h"
#include "RankTwoTensor.h"
#include "MooseMesh.h"

registerMooseObject("NavierStokesApp", INSEshelbyTensor);

InputParameters
INSEshelbyTensor::validParams()
{
  InputParameters params = Material::validParams();
  params.addClassDescription("Computes the Eshelby tensor as a function of "
                             "strain energy density and the first "
                             "Piola-Kirchhoff stress");
  params.addRequiredCoupledVar(
      "velocity",
      "The velocities appropriate for the simulation geometry and coordinate system");
  params.addParam<std::string>("base_name",
                               "Optional parameter that allows the user to define "
                               "multiple mechanics material systems on the same "
                               "block, i.e. for multiple phases");
  params.addParam<bool>(
      "compute_dissipation",
      false,
      "Whether to compute Eshelby tensor's dissipation (or rate of change). This tensor"
      "yields the increase in dissipation per unit crack advanced");
  params.addCoupledVar("temperature", "Coupled temperature");
  return params;
}

INSEshelbyTensor::INSEshelbyTensor(const InputParameters & parameters)
  : DerivativeMaterialInterface<Material>(parameters),
    _base_name(isParamValid("base_name") ? getParam<std::string>("base_name") + "_" : ""),
    _compute_dissipation(getParam<bool>("compute_dissipation")),
    // _sed(getMaterialPropertyByName<Real>(_base_name + "strain_energy_density")),
    _serd(_compute_dissipation
              ? &getMaterialPropertyByName<Real>(_base_name + "strain_energy_rate_density")
              : nullptr),
    // _eshelby_tensor(declareProperty<RankTwoTensor>(_base_name + "Eshelby_tensor")),
    _eshelby_tensor_dissipation(
        _compute_dissipation
            ? &declareProperty<RankTwoTensor>(_base_name + "Eshelby_tensor_dissipation")
            : nullptr),
    _stress(getMaterialProperty<RankTwoTensor>(_base_name + "stress")),
    // _stress_old(getMaterialPropertyOld<RankTwoTensor>(_base_name + "stress")),
    _velocity(coupledVectorValue("velocity")),
    _grad_velocity(coupledVectorGradient("velocity")),
    // _grad_disp(3),
    // _grad_disp_old(3),
    _J_thermal_term_vec(declareProperty<RealVectorValue>("J_thermal_term_vec")),
    _grad_temp(coupledGradient("temperature")),
    _has_temp(isCoupled("temperature")),
    _total_deigenstrain_dT(hasMaterialProperty<RankTwoTensor>("total_deigenstrain_dT")
                               ? &getMaterialProperty<RankTwoTensor>("total_deigenstrain_dT")
                               : nullptr)
{
  // unsigned int ndisp = coupledComponents("displacements");
  unsigned int ndisp = 2;
  // Checking for consistency between mesh size and length of the provided displacements vector
  if (ndisp != _mesh.dimension())
    mooseError(
        "The number of variables supplied in 'displacements' must match the mesh dimension.");

  // // fetch coupled gradients
  // for (unsigned int i = 0; i < ndisp; ++i)
  //   _grad_disp[i] = &coupledGradient("displacements", i);
  //
  // // set unused dimensions to zero
  // for (unsigned i = ndisp; i < 3; ++i)
  //   _grad_disp[i] = &_grad_zero;

  // WE DONT NEED TO CALCULATE ANYTHING "OLD" BECAUSE ITS A STEADY SOLUTION
  // // Need previos step's displacements to compute deformation gradient time rate
  // if (_compute_dissipation)
  // {
  //   // fetch coupled gradients previous step
  //   for (unsigned int i = 0; i < ndisp; ++i)
  //     _grad_disp_old[i] = &coupledGradientOld("displacements", i);
  //
  //   // set unused dimensions to zero
  //   for (unsigned i = ndisp; i < 3; ++i)
  //     _grad_disp_old[i] = &_grad_zero;
  // }

  if (_has_temp && !_total_deigenstrain_dT)
    mooseError("INSEshelbyTensor Error: To include thermal strain term in Fracture integral "
               "calculation, must both couple temperature in DomainIntegral block and compute "
               "total_deigenstrain_dT using ThermalFractureIntegral material model.");
}

void
INSEshelbyTensor::initQpStatefulProperties()
{
}

void
INSEshelbyTensor::computeQpProperties()
{
  // RankTwoTensor F = _stress[_qp] * 0.0;
  RankTwoTensor F = _grad_velocity[_qp] * 1.0e8;
  // Moose::out<<"grad V(0,0): "<<_grad_velocity[_qp](0,0)<<"\n";
  // Moose::out<<"F(0,0): "<<F(0,0)<<"\n";

  // F(0,0) = _velocity[_qp](0) * 1.0e8;
  // F(1,1) = _velocity[_qp](1) * 1.0e8;
  // F(2,2) = _velocity[_qp](2) * 1.0e8;
  // RankTwoTensor F((*_grad_disp[0])[_qp],
  //                 (*_grad_disp[1])[_qp],
  //                 (*_grad_disp[2])[_qp]); // Deformation gradient

  RankTwoTensor H(F);
  F.addIa(1.0);
  Real detF = F.det();
  RankTwoTensor FinvT(F.inverse().transpose());

  // 1st Piola-Kirchoff Stress (P):
  RankTwoTensor P = detF * _stress[_qp] * FinvT;

  // HTP = H^T * P = H^T * detF * sigma * FinvT;
  // RankTwoTensor HTP = H.transpose() * P;

  // RankTwoTensor WI = RankTwoTensor(RankTwoTensor::initIdentity);
  // WI *= (_sed[_qp] * detF);

  // _eshelby_tensor[_qp] = WI - HTP;

  // Compute deformation gradient rate (this is the velocity gradient)
  if (_compute_dissipation == true)
  {
    // RankTwoTensor H_old(
    //     (*_grad_disp_old[0])[_qp], (*_grad_disp_old[1])[_qp], (*_grad_disp_old[2])[_qp]);

    RankTwoTensor Wdot = RankTwoTensor(RankTwoTensor::initIdentity);
    Wdot *= ((*_serd)[_qp] * detF);

    // // F_dot = (F - F_old)/dt
    // RankTwoTensor F_dot = (H - H_old) / _dt;
    RankTwoTensor F_dot = _grad_velocity[_qp];
    // Moose::out<<"F_dot: "<<F_dot(0,0)<<"\n";

    // FdotTP = Fdot^T * P = Fdot^T * detF * sigma * FinvT;
    RankTwoTensor FdotTP = F_dot.transpose() * P;

    (*_eshelby_tensor_dissipation)[_qp] = Wdot - FdotTP;
    // Moose::out<<"eshelby_tensor_dissipation: "<<Wdot - FdotTP<<"\n";
  }

  if (_has_temp)
  {
    Real sigma_alpha = _stress[_qp].doubleContraction((*_total_deigenstrain_dT)[_qp]);
    _J_thermal_term_vec[_qp] = sigma_alpha * _grad_temp[_qp];
  }
  else
    _J_thermal_term_vec[_qp].zero();
}
