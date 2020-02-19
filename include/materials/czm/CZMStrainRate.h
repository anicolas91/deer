//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "InterfaceMaterial.h"

/**
 * use the displacement jump rate to compute the itnerface strainrate
 */
class CZMStrainRate : public InterfaceMaterial {
public:
  static InputParameters validParams();
  CZMStrainRate(const InputParameters &parameters);

protected:
  virtual void computeQpProperties() override;
  // virtual void initQpStatefulProperties() override;

  /// normal to the interface
  const MooseArray<Point> &_normals;

  /// number of displacement components
  const unsigned int _ndisp;

  /// the coupled displacement and neighbor displacement values
  ///@{
  std::vector<const VariableValue *> _disp;
  std::vector<const VariableValue *> _disp_neighbor;
  ///@}

  /// the value of the interface strain
  ///@{
  MaterialProperty<RankTwoTensor> &_interface_strain;
  MaterialProperty<RankTwoTensor> &_interface_opening_strain;
  MaterialProperty<RankTwoTensor> &_interface_sliding_strain;
  MaterialProperty<RankTwoTensor> &_interface_strain_rate;
  MaterialProperty<RankTwoTensor> &_interface_opening_strain_rate;
  MaterialProperty<RankTwoTensor> &_interface_sliding_strain_rate;
  ///@}
};
