#pragma once

#include "AuxKernel.h"

class CurlTensorAux;

template <>
InputParameters validParams<CurlTensorAux>();

class CurlTensorAux : public AuxKernel
{
 public:
  CurlTensorAux(const InputParameters & params);

 protected:
  virtual void initialSetup() override;
  virtual Real computeValue() override;

  unsigned int _nvar;
  std::vector<const VariableGradient *> _grad_vars;
  unsigned int _i, _j;
  unsigned int _ndim;
};
