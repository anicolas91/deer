#pragma once

#include "Action.h"

class CZMRVEStrainAction : public Action {
public:
  static InputParameters validParams();
  CZMRVEStrainAction(const InputParameters &params);

  virtual void act() override;

protected:
  const std::vector<VariableName> _displacements;
  const int _n_disp;
  const std::vector<BoundaryName> _boundary_names;
  const std::vector<SubdomainName> _grains_block_names;

  void addTensorPostProcessorBulk(const std::string /*mp_name*/);
  void addTensorPostProcessorInterface(const std::string /*mp_name*/);
  void sumStrain(const std::string /*mp_name*/);
};
