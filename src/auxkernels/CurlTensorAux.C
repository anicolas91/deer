#include "CurlTensorAux.h"

registerMooseObject("DeerApp", CurlTensorAux);

template <>
InputParameters
validParams<CurlTensorAux>()
{
  InputParameters params = validParams<AuxKernel>();

  params.addRequiredParam<unsigned int>("i", "First index");
  params.addRequiredParam<unsigned int>("j", "Second index");

  params.addRequiredCoupledVar("components", "Each component of the tensor to operate on");

  return params;
}

CurlTensorAux::CurlTensorAux(const InputParameters & parameters)
  : AuxKernel(parameters),
    _nvar(coupledComponents("components")),
    _grad_vars(_nvar),
    _i(getParam<unsigned int>("i")),
    _j(getParam<unsigned int>("j"))

{

}

void
CurlTensorAux::initialSetup()
{
  if (_nvar != 9) {
    throw MooseException("Expecting 9 variables representing a row major unrolling of a rank 2 tensor.");
  }
  for (size_t i = 0; i < _nvar; i++) {
    _grad_vars[i] = &coupledGradient("components", i);
  }
}

Real
CurlTensorAux::computeValue()
{
  auto imap = [](unsigned int i, unsigned int j) { return i*3+j; };
  auto lciv = [](unsigned int i, unsigned int j, unsigned int k) {
    return (i-j)*(j-k)*(k-1)/2;};
  
  Real val = 0.0;
  for (unsigned int i = 0; i < 3; i++) {
    for (unsigned int j = 0; j < 3; j++) {
      val += lciv(i,j,_i) * (*_grad_vars[imap(_j,j)])[_qp](i);
    }
  }
   
  return val;
}
