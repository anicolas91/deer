//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SumPostprocessor.h"

registerMooseObject("MooseApp", SumPostprocessor);

InputParameters SumPostprocessor::validParams() {
  InputParameters params = DifferencePostprocessor::validParams();
  params.addClassDescription("This postprocessor computes computes teh sum of "
                             "the value computed by two postprocessors");
  return params;
}

SumPostprocessor::SumPostprocessor(const InputParameters &parameters)
    : DifferencePostprocessor(parameters) {}

PostprocessorValue SumPostprocessor::getValue() { return _value1 + _value2; }
