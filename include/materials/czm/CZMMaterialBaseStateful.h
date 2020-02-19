//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "CZMMaterialBase.h"

/**
 *
 */
class CZMMaterialBaseStateful : public CZMMaterialBase {
public:
  static InputParameters validParams();
  CZMMaterialBaseStateful(const InputParameters &params);

protected:
  virtual void computeTractionDerivativesAndNewMaterialPropertiesLocal() = 0;
  virtual void computeQpProperties() override;
  virtual void initQpStatefulProperties() override;
  /// the coupled displacement and neighbor displacement rates
  ///@{
  std::vector<const VariableValue *> _disp_dot;
  std::vector<const VariableValue *> _disp_neighbor_dot;
  ///@}

  /// the old displacement jump in global and local coordiante
  ///@{
  const MaterialProperty<RealVectorValue> &_displacement_jump_global_old;
  const MaterialProperty<RealVectorValue> &_displacement_jump_old;
  ///@}

  /// the old value of the traction in global and local coordinates
  ///@{
  const MaterialProperty<RealVectorValue> &_traction_global_old;
  const MaterialProperty<RealVectorValue> &_traction_old;
  ///@}

  /// the displacement jump rate in global and local coordiante
  ///@{
  MaterialProperty<RealVectorValue> &_displacement_jump_dot_global;
  MaterialProperty<RealVectorValue> &_displacement_jump_dot;
  ///@}

  /// the old displacement jump rate in global and local coordiante
  ///@{
  const MaterialProperty<RealVectorValue> &_displacement_jump_dot_global_old;
  const MaterialProperty<RealVectorValue> &_displacement_jump_dot_old;
  ///@}
};
