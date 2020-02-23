#pragma once

#include "f_Low.h"

f_Low::f_Low(const std::string &fname, const f_aL &faL, const f_ab &fab)
    : nlFunBase(fname, requiredVar(), requiredParam()), _faL(faL), _fab(fab),
      _soft_max_min("soft_max_min", 100,
                    /*max_of_abs =*/false,
                    /*const bool &max_of_abs_signed = */ false,
                    /*const bool &compute_min = */ false),
      _soft_max("soft_max") {}

// f_Low::f_Low(const std::string &fname, const Real &D)
//     : nlFunBase(fname, requiredVar(), requiredParam()), _faL(f_aL("faL", D)),
//       _fab(f_ab("fab")), _soft_max(softMaxMin("fmax")) {}

Real f_Low::computeValue(const io_maps_type &x, const io_maps_type &params,
                         const io_maps_type &x_old) const {
  UNUSED(x_old);

  /*compute x f old*/

  Real fal = _faL.computeValue(x, params, x_old);
  Real fab = _fab.computeValue(x, params, x_old);

  return _soft_max.computeSoftMaxValue(fal, fab);
  // if (fal_old > fab_old)
  //   return _fab.computeValue(x, params, x_old);
  // else
  //   return _fab.computeValue(x, params, x_old);

  return 0;
}

f_Low::io_maps_type f_Low::computeVarGradient(const io_maps_type &x,
                                              const io_maps_type &params,
                                              const io_maps_type &x_old) const {

  // io_maps_type f;
  // map_of_io_maps_type f_grad;
  // f["aL"] = _faL.computeValue(x, params, x_old);
  // f["ab"] = _fab.computeValue(x, params, x_old);
  //
  // io_maps_type dfal_dp = _faL.computeVarGradient(x, params, x_old);
  // io_maps_type dfab_dp = _fab.computeVarGradient(x, params, x_old);
  // f_grad = {{"aL", &dfal_dp}, {"ab", &dfab_dp}};
  // return _soft_max.computeVarGradient(f, f_grad, params);

  /*compute x f old*/

  Real fal = _faL.computeValue(x, params, x_old);
  Real fab = _fab.computeValue(x, params, x_old);
  io_maps_type dfaL_dx = _faL.computeVarGradient(x, params, x_old);
  io_maps_type dfab_dx = _fab.computeVarGradient(x, params, x_old);

  io_maps_type dmax_dfi = _soft_max.computeSoftMaxGradient(fal, fab);

  Real dmax_dfal = dmax_dfi.find("a")->second;
  Real dmax_dfab = dmax_dfi.find("b")->second;
  dfaL_dx = chain(dmax_dfal, dfaL_dx);
  dfab_dx = chain(dmax_dfab, dfab_dx);
  return sumD(dfaL_dx, dfab_dx);

  // if (fal_old > fab_old)
  //   return _fab.computeVarGradient(x, params, x_old);
  // else
  //   return _fab.computeVarGradient(x, params, x_old);
}

f_Low::io_maps_type
f_Low::computeParamGradient(const io_maps_type &x, const io_maps_type &params,
                            const io_maps_type &x_old) const {
  UNUSED(x_old);

  // io_maps_type f;
  // map_of_io_maps_type f_grad;
  // f["aL"] = _faL.computeValue(x, params, x_old);
  // f["ab"] = _fab.computeValue(x, params, x_old);
  //
  // io_maps_type dfal_dp = _faL.computeParamGradient(x, params, x_old);
  // io_maps_type dfab_dp = _fab.computeParamGradient(x, params, x_old);
  //
  // f_grad = {{"aL", &dfal_dp}, {"ab", &dfab_dp}};
  //
  // return _soft_max.computeParamGradient(f, f_grad, params);
  //
  // Real fal = _faL.computeValue(x, params, x_old);
  // Real fab = _fab.computeValue(x, params, x_old);
  //
  // if (fal > fab)
  //   return _faL.computeParamGradient(x, params, x_old);
  // else
  //   return _fab.computeParamGradient(x, params, x_old);

  return _empty_map;
}
