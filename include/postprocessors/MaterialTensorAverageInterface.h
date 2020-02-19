//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "InterfaceAveragePostprocessor.h"
#include "RankTwoTensor.h"

/**
 * This postprocessor computes an element Average of a
 * component of a material tensor as specified by the user-supplied indices.
 */
class MaterialTensorAverageInterface : public InterfaceAveragePostprocessor {
public:
  static InputParameters validParams();
  MaterialTensorAverageInterface(const InputParameters &parameters);

protected:
  virtual Real computeQpIntegral();

private:
  const MaterialProperty<RankTwoTensor> &_tensor;
  const unsigned int _i;
  const unsigned int _j;
};
