//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MaterialTensorAverageInterface.h"
#include "RankTwoScalarTools.h"

registerMooseObject("DeerApp", MaterialTensorAverageInterface);

InputParameters MaterialTensorAverageInterface::validParams() {
  InputParameters params = InterfaceAveragePostprocessor::validParams();
  params.addClassDescription(
      "This postprocessor computes the average of "
      "a component of an interface material tensor as specified by "
      "the user-supplied indices");
  params.addRequiredParam<MaterialPropertyName>(
      "rank_two_tensor", "The rank two material tensor name");
  params.addRequiredRangeCheckedParam<unsigned int>(
      "index_i", "index_i >= 0 & index_i <= 2",
      "The index i of ij for the tensor to output (0, 1, 2)");
  params.addRequiredRangeCheckedParam<unsigned int>(
      "index_j", "index_j >= 0 & index_j <= 2",
      "The index j of ij for the tensor to output (0, 1, 2)");

  params.set<bool>("use_displaced_mesh") = true;
  return params;
}

MaterialTensorAverageInterface::MaterialTensorAverageInterface(
    const InputParameters &parameters)
    : InterfaceAveragePostprocessor(parameters),
      _tensor(getMaterialProperty<RankTwoTensor>("rank_two_tensor")),
      _i(getParam<unsigned int>("index_i")),
      _j(getParam<unsigned int>("index_j")) {}

Real MaterialTensorAverageInterface::computeQpIntegral() {
  return _tensor[_qp](_i, _j);
}
