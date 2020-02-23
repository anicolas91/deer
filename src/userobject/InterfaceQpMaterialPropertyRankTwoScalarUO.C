//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "InterfaceQpMaterialPropertyRankTwoScalarUO.h"

registerMooseObject("DeerApp", InterfaceQpMaterialPropertyRankTwoScalarUO);

defineLegacyParams(InterfaceQpMaterialPropertyRankTwoScalarUO);

InputParameters InterfaceQpMaterialPropertyRankTwoScalarUO::validParams() {
  InputParameters params =
      InterfaceQpMaterialPropertyBaseUserObject<RankTwoTensor>::validParams();
  params.addParam<MooseEnum>("scalar_type", RankTwoScalarTools::scalarOptions(),
                             "Type of scalar output");
  params.addClassDescription(
      "Computes the value, rate or increment of a RankTwoTensor Material "
      "property across an "
      "interface. The value or rate is computed according to the provided "
      "interface_value_type parameter");

  return params;
}

InterfaceQpMaterialPropertyRankTwoScalarUO::
    InterfaceQpMaterialPropertyRankTwoScalarUO(
        const InputParameters &parameters)
    : InterfaceQpMaterialPropertyBaseUserObject<RankTwoTensor>(parameters),
      _scalar_type(getParam<MooseEnum>("scalar_type")), _point1(Point(0, 0, 0)),
      _point2(Point(0, 1, 0)), _input_direction(Point(0, 0, 1))

{}
