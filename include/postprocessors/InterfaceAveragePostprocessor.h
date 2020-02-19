//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "InterfaceIntegralPostprocessor.h"

/**
 * This postprocessor computes a volume integral of the specified variable.
 *
 * Note that specializations of this integral are possible by deriving from this
 * class and overriding computeQpIntegral().
 */
class InterfaceAveragePostprocessor : public InterfaceIntegralPostprocessor {
public:
  static InputParameters validParams();
  InterfaceAveragePostprocessor(const InputParameters &parameters);

  virtual Real getValue() override;
  const unsigned int _n_volumes;
  std::vector<const PostprocessorValue *> _volumes_pp;
};
