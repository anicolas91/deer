//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "InterfaceAveragePostprocessor.h"

#include "libmesh/quadrature.h"

InputParameters InterfaceAveragePostprocessor::validParams() {
  InputParameters params = InterfaceIntegralPostprocessor::validParams();
  params.addParam<std::vector<PostprocessorName>>(
      "volumes_PP",
      "a vector of postprocessor computing the volume of the system");
  return params;
}

InterfaceAveragePostprocessor::InterfaceAveragePostprocessor(
    const InputParameters &parameters)
    : InterfaceIntegralPostprocessor(parameters),
      _n_volumes(
          parameters.isParamSetByUser("volumes_PP")
              ? getParam<std::vector<PostprocessorName>>("volumes_PP").size()
              : 0),
      _volumes_pp(_n_volumes) {

  if (_n_volumes > 0) {
    auto volume_pp_names =
        parameters.get<std::vector<PostprocessorName>>("volumes_PP");
    for (unsigned int i = 0; i < _n_volumes; ++i)
      _volumes_pp[i] = &getPostprocessorValueByName(volume_pp_names[i]);
  }
}

Real InterfaceAveragePostprocessor::getValue() {
  InterfaceIntegralPostprocessor::getValue();

  Real avg;
  if (_n_volumes > 0) {
    Real vtot = 0;
    for (unsigned int i = 0; i < _n_volumes; ++i)
      vtot += (*_volumes_pp[i]);
    avg = _integral_value / vtot;
  } else
    avg = _integral_value / _interface_master_area;

  return avg;
}
