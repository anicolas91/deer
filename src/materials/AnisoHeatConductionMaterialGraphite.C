//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AnisoHeatConductionMaterialGraphite.h"
#include "Function.h"

#include "libmesh/quadrature.h"

registerMooseObject("DeerApp", AnisoHeatConductionMaterialGraphite);

template <>
InputParameters
validParams<AnisoHeatConductionMaterialGraphite>()
{
  InputParameters params = validParams<Material>();

  params.addCoupledVar("temp", "Coupled Temperature");

  params.addParam<Real>("thermal_conductivity_x", "The thermal conductivity value in the x direction");
  params.addParam<Real>("thermal_conductivity_y", "The thermal conductivity value in the y direction");
  params.addParam<Real>("thermal_conductivity_z", "The thermal conductivity value in the z direction");

  params.addParam<FunctionName>("thermal_conductivity_x_temperature_function",
                                "",
                                "Thermal conductivity in x direction as a function of temperature.");
  params.addParam<FunctionName>("thermal_conductivity_y_temperature_function",
                                "",
                                "Thermal conductivity in y direction as a function of temperature.");
  params.addParam<FunctionName>("thermal_conductivity_z_temperature_function",
                                "",
                                "Thermal conductivity in z direction as a function of temperature.");

  params.addParam<Real>("specific_heat", "The specific heat value");
  params.addParam<FunctionName>(
      "specific_heat_temperature_function", "", "Specific heat as a function of temperature.");
  params.addClassDescription("General-purpose material model for heat conduction");

  return params;
}

AnisoHeatConductionMaterialGraphite::AnisoHeatConductionMaterialGraphite(const InputParameters & parameters)
  : Material(parameters),

    _has_temp(isCoupled("temp")),
    _temperature(_has_temp ? coupledValue("temp") : _zero),
    _my_thermal_conductivity_x(
        isParamValid("thermal_conductivity_x") ? getParam<Real>("thermal_conductivity_x") : 0),
    _my_thermal_conductivity_y(
        isParamValid("thermal_conductivity_y") ? getParam<Real>("thermal_conductivity_y") : 0),
    _my_thermal_conductivity_z(
        isParamValid("thermal_conductivity_z") ? getParam<Real>("thermal_conductivity_z") : 0),

    _my_specific_heat(isParamValid("specific_heat") ? getParam<Real>("specific_heat") : 0),

    _thermal_conductivity_x(declareProperty<Real>("thermal_conductivity_x")),
    _thermal_conductivity_x_dT(declareProperty<Real>("thermal_conductivity_x_dT")),
    _thermal_conductivity_x_temperature_function(
        getParam<FunctionName>("thermal_conductivity_x_temperature_function") != ""
            ? &getFunction("thermal_conductivity_x_temperature_function")
            : NULL),
    _thermal_conductivity_y(declareProperty<Real>("thermal_conductivity_y")),
    _thermal_conductivity_y_dT(declareProperty<Real>("thermal_conductivity_y_dT")),
    _thermal_conductivity_y_temperature_function(
        getParam<FunctionName>("thermal_conductivity_y_temperature_function") != ""
            ? &getFunction("thermal_conductivity_y_temperature_function")
            : NULL),
    _thermal_conductivity_z(declareProperty<Real>("thermal_conductivity_z")),
    _thermal_conductivity_z_dT(declareProperty<Real>("thermal_conductivity_z_dT")),
    _thermal_conductivity_z_temperature_function(
        getParam<FunctionName>("thermal_conductivity_z_temperature_function") != ""
            ? &getFunction("thermal_conductivity_z_temperature_function")
            : NULL),

    _specific_heat(declareProperty<Real>("specific_heat")),
    _specific_heat_temperature_function(
        getParam<FunctionName>("specific_heat_temperature_function") != ""
            ? &getFunction("specific_heat_temperature_function")
            : NULL)
{
  if (_thermal_conductivity_x_temperature_function && !_has_temp)
  {
    mooseError("Must couple with temperature if using thermal conductivity function");
  }
  if (isParamValid("thermal_conductivity_x") && _thermal_conductivity_x_temperature_function)
  {
    mooseError(
        "Cannot define both thermal conductivity and thermal conductivity temperature function");
  }
  if (_specific_heat_temperature_function && !_has_temp)
  {
    mooseError("Must couple with temperature if using specific heat function");
  }
  if (isParamValid("specific_heat") && _specific_heat_temperature_function)
  {
    mooseError("Cannot define both specific heat and specific heat temperature function");
  }
}

void
AnisoHeatConductionMaterialGraphite::computeProperties()
{
  for (unsigned int qp(0); qp < _qrule->n_points(); ++qp)
  {
    Real qp_temperature = 0;
    if (_has_temp)
    {
      qp_temperature = _temperature[qp];
      if (_temperature[qp] < 0)
      {
        std::stringstream msg;
        msg << "WARNING:  In AnisoHeatConductionMaterialGraphite:  negative temperature!\n"
            << "\tResetting to zero.\n"
            << "\t_qp: " << qp << "\n"
            << "\ttemp: " << _temperature[qp] << "\n"
            << "\telem: " << _current_elem->id() << "\n"
            << "\tproc: " << processor_id() << "\n";
        mooseWarning(msg.str());
        qp_temperature = 0;
      }
    }
    if (_thermal_conductivity_x_temperature_function)
    {
      Point p;
      _thermal_conductivity_x[qp] =
          _thermal_conductivity_x_temperature_function->value(qp_temperature, p);
      _thermal_conductivity_x_dT[qp] =
          _thermal_conductivity_x_temperature_function->timeDerivative(qp_temperature, p);
    }
    else
    {
      _thermal_conductivity_x[qp] = _my_thermal_conductivity_x;
      _thermal_conductivity_x_dT[qp] = 0;
    }

    if (_thermal_conductivity_y_temperature_function)
    {
      Point p;
      _thermal_conductivity_y[qp] =
          _thermal_conductivity_y_temperature_function->value(qp_temperature, p);
      _thermal_conductivity_y_dT[qp] =
          _thermal_conductivity_y_temperature_function->timeDerivative(qp_temperature, p);
    }
    else
    {
      _thermal_conductivity_y[qp] = _my_thermal_conductivity_y;
      _thermal_conductivity_y_dT[qp] = 0;
    }

    if (_thermal_conductivity_z_temperature_function)
    {
      Point p;
      _thermal_conductivity_z[qp] =
          _thermal_conductivity_z_temperature_function->value(qp_temperature, p);
      _thermal_conductivity_z_dT[qp] =
          _thermal_conductivity_z_temperature_function->timeDerivative(qp_temperature, p);
    }
    else
    {
      _thermal_conductivity_z[qp] = _my_thermal_conductivity_z;
      _thermal_conductivity_z_dT[qp] = 0;
    }

    if (_specific_heat_temperature_function)
    {
      Point p;
      _specific_heat[qp] = _specific_heat_temperature_function->value(qp_temperature, p);
    }
    else
    {
      _specific_heat[qp] = _my_specific_heat;
    }
  }
}
