//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "InterfaceQpMaterialPropertyBaseUserObject.h"
#include "RankTwoScalarTools.h"

/**
 * Specialization of InterfaceQpMaterialPropertyRankTwoScalarUO for Real
 * material properties.
 */
class InterfaceQpMaterialPropertyRankTwoScalarUO
    : public InterfaceQpMaterialPropertyBaseUserObject<RankTwoTensor> {

public:
  static InputParameters validParams();
  /**
   * Class constructor
   * @param parameters The input parameters for this object
   */
  InterfaceQpMaterialPropertyRankTwoScalarUO(const InputParameters &parameters);
  virtual ~InterfaceQpMaterialPropertyRankTwoScalarUO(){};

protected:
  MooseEnum _scalar_type;

  const Point _point1;
  const Point _point2;
  Point _input_direction;

  /// return the material property value at the give qp
  virtual Real
  computeScalarMaterialProperty(const MaterialProperty<RankTwoTensor> *p,
                                const unsigned int qp) override final {
    return RankTwoScalarTools::getQuantity((*p)[qp], _scalar_type, _point1,
                                           _point2, _q_point[qp],
                                           _input_direction);
  }
};
