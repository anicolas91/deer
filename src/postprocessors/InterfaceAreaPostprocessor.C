//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "InterfaceAreaPostprocessor.h"

#include "libmesh/quadrature.h"

registerMooseObject("DeerApp", InterfaceAreaPostprocessor);

InputParameters InterfaceAreaPostprocessor::validParams() {
  InputParameters params = InterfacePostprocessor::validParams();
  return params;
}

InterfaceAreaPostprocessor::InterfaceAreaPostprocessor(
    const InputParameters &parameters)
    : InterfacePostprocessor(parameters) {}

Real InterfaceAreaPostprocessor::getValue() {
  InterfacePostprocessor::getValue();
  return _interface_master_area;
}
