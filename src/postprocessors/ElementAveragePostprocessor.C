//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ElementAveragePostprocessor.h"

#include "libmesh/quadrature.h"

InputParameters ElementAveragePostprocessor::validParams() {
  InputParameters params = ElementIntegralPostprocessor::validParams();
  params.addParam<std::vector<PostprocessorName>>(
      "volumes_PP",
      "a vector of postprocessor computing the volume of the system");
  return params;
}

ElementAveragePostprocessor::ElementAveragePostprocessor(
    const InputParameters &parameters)
    : ElementIntegralPostprocessor(parameters), _volume(0),
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
void ElementAveragePostprocessor::initialize() {
  ElementIntegralPostprocessor::initialize();
  _volume = 0;
}

void ElementAveragePostprocessor::execute() {
  ElementIntegralPostprocessor::execute();
  _volume += _current_elem_volume;
}

Real ElementAveragePostprocessor::getValue() {
  Real integral = ElementIntegralPostprocessor::getValue();
  gatherSum(_volume);

  Real avg;
  if (_n_volumes > 0) {
    Real vtot = 0;
    for (unsigned int i = 0; i < _n_volumes; ++i)
      vtot += (*_volumes_pp[i]);
    avg = integral / vtot;
  } else
    avg = integral / _volume;
  return avg;
}

void ElementAveragePostprocessor::threadJoin(const UserObject &y) {
  ElementIntegralPostprocessor::threadJoin(y);
  const ElementAveragePostprocessor &pps =
      static_cast<const ElementAveragePostprocessor &>(y);
  _volume += pps._volume;
}
