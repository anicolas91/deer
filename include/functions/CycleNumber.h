//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Function.h"

class CycleNumber;

template <>
InputParameters validParams<CycleNumber>();

class CycleNumber : public Function
{
public:
  CycleNumber(const InputParameters & parameters);

  Real value(Real t, const Point &p) const override;

  const Real _cycle_period;
};