#pragma once

#include "Action.h"

class NEMLCPAction;

template <>
InputParameters validParams<NEMLCPAction>();

class NEMLCPAction: public Action
{
 public:
  NEMLCPAction(const InputParameters & params);

  virtual void act();

 private:
  void _add_full_tensor_variable(std::string name, bool gradient = false);
  void _add_tensor_variable(std::string name, bool gradient = false);
  void _add_scalar_variable(std::string name, bool gradient = false);
  void _add_curl_full_tensor_aux(std::string invar, std::string outvar);
  void _add_full_tensor_aux(std::string name);
  void _add_tensor_aux(std::string name);
  void _add_scalar_aux(std::string name);
  void _add_vector_aux(std::string name, size_t index, std::string vname);

 private:
  bool _add_orientation_output;
  bool _add_fe;
  bool _add_nye;
};

