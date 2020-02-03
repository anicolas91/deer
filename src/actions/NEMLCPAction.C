#include "NEMLCPAction.h"

#include "AddVariableAction.h"
#include "FEProblem.h"

registerMooseAction("DeerApp", NEMLCPAction, "add_aux_variable");
registerMooseAction("DeerApp", NEMLCPAction, "add_aux_kernel");

const std::map<std::pair<int, int>, std::string> tensor_map_cp = {
    {std::make_pair(0, 0), "x-x"}, {std::make_pair(1, 1), "y-y"},
    {std::make_pair(2, 2), "z-z"}, {std::make_pair(0, 1), "x-y"},
    {std::make_pair(0, 2), "x-z"}, {std::make_pair(1, 2), "y-z"}};

const std::map<std::pair<int, int>, std::string> full_tensor_map_cp = {
    {std::make_pair(0, 0), "x-x"}, {std::make_pair(0, 1), "x-y"}, {std::make_pair(0,2), "x-z"},
    {std::make_pair(1, 0), "y-x"}, {std::make_pair(1, 1), "y-y"}, {std::make_pair(1,2), "y-z"},
    {std::make_pair(2, 0), "z-x"}, {std::make_pair(2, 1), "z-y"}, {std::make_pair(2,2), "z-z"}};

template <> InputParameters validParams<NEMLCPAction>() {
  InputParameters params = validParams<Action>();

  params.addParam<bool>("add_orientation_output", true,
                        "Outputs the quaternion orientation in active convention");
  params.addParam<bool>("add_Fe", false,
                        "Outputs the elastic part of the deformation gradient.");
  params.addParam<bool>("add_nye", false,
                        "Compute and output the Nye tensor from Fe.");

  return params;
}

NEMLCPAction::NEMLCPAction(const InputParameters & params) :
    Action(params), 
    _add_orientation_output(getParam<bool>("add_orientation_output")),
    _add_fe(getParam<bool>("add_Fe") or getParam<bool>("add_nye")),
    _add_nye(getParam<bool>("add_nye"))
{

}

void NEMLCPAction::act() {
  if (_current_task == "add_aux_variable") {
    if (_add_orientation_output) {
      for (size_t i=0; i<4; i++) {
        _add_scalar_variable("q_"+std::to_string(i));
      }
    }
    if (_add_fe) {
      _add_full_tensor_variable("Fe", _add_nye);
    }
    if (_add_nye) {
      _add_full_tensor_variable("nye");
    }
  }
  else if (_current_task == "add_aux_kernel") {
    if (_add_orientation_output) {
      for (size_t i=0; i<4; i++) {
        _add_vector_aux("orientation_q", i, "q_"+std::to_string(i));
      }
    }
    if (_add_fe) {
      _add_full_tensor_aux("Fe", "nonlinear");
    }
    if (_add_nye) {
      _add_curl_full_tensor_aux("Fe", "nye", "nonlinear");
    }
  }
}

void NEMLCPAction::_add_full_tensor_variable(std::string name, bool gradient) {
  for (auto entry : full_tensor_map_cp) {
    _add_scalar_variable(name + "_" + entry.second, gradient);
  }
}

void NEMLCPAction::_add_tensor_variable(std::string name, bool gradient) {
  for (auto entry : tensor_map_cp) {
    _add_scalar_variable(name + "_" + entry.second, gradient);
  }
}

void NEMLCPAction::_add_scalar_variable(std::string name, bool gradient) {

  InputParameters params = _factory.getValidParams("MooseVariableBase");
  
  if (gradient) {
    params.set<MooseEnum>("family") = "MONOMIAL";
    params.set<MooseEnum>("order") = "FIRST";
  }
  else {
    params.set<MooseEnum>("family") = "MONOMIAL";
    params.set<MooseEnum>("order") = "CONSTANT";
  }

  auto fe_type = AddVariableAction::feType(params);
  auto var_type = AddVariableAction::determineType(fe_type, 1);

  _problem->addAuxVariable(var_type, name, params);
}

void NEMLCPAction::_add_curl_full_tensor_aux(std::string invar, std::string outvar,
                                             std::string when)
{
  std::vector<VariableName> varnames;
  for (unsigned int i = 0; i < 3; i++) {
    for (unsigned int j = 0; j < 3; j++) {
      varnames.push_back(invar + "_" + full_tensor_map_cp.at(std::make_pair(i,j)));
    }
  }

  for (auto entry : full_tensor_map_cp) {
    InputParameters params = _factory.getValidParams("CurlTensorAux");

    params.set<std::vector<VariableName>>("components") = varnames;

    params.set<AuxVariableName>("variable") = outvar + "_" + entry.second;
    params.set<unsigned int>("i") = entry.first.first;
    params.set<unsigned int>("j") = entry.first.second;

    params.set<ExecFlagEnum>("execute_on") = when;

    _problem->addAuxKernel("CurlTensorAux", outvar + "_" + entry.second, params);
  }
}

void NEMLCPAction::_add_full_tensor_aux(std::string name, std::string when) {
  for (auto entry : full_tensor_map_cp) {
    auto params = _factory.getValidParams("RankTwoAux");

    params.set<MaterialPropertyName>("rank_two_tensor") = name;
    params.set<AuxVariableName>("variable") = name + "_" + entry.second;
    params.set<unsigned int>("index_i") = entry.first.first;
    params.set<unsigned int>("index_j") = entry.first.second;

    params.set<ExecFlagEnum>("execute_on") = when;

    _problem->addAuxKernel("RankTwoAux", name + "_" + entry.second, params);
  }
}

void NEMLCPAction::_add_tensor_aux(std::string name, std::string when) {
  for (auto entry : tensor_map_cp) {
    auto params = _factory.getValidParams("RankTwoAux");

    params.set<MaterialPropertyName>("rank_two_tensor") = name;
    params.set<AuxVariableName>("variable") = name + "_" + entry.second;
    params.set<unsigned int>("index_i") = entry.first.first;
    params.set<unsigned int>("index_j") = entry.first.second;

    params.set<ExecFlagEnum>("execute_on") = when;

    _problem->addAuxKernel("RankTwoAux", name + "_" + entry.second, params);
  }
}

void NEMLCPAction::_add_scalar_aux(std::string name, std::string when) {
  auto params = _factory.getValidParams("MaterialRealAux");

  params.set<MaterialPropertyName>("property") = name;
  params.set<AuxVariableName>("variable") = name;

  params.set<ExecFlagEnum>("execute_on") = when;

  _problem->addAuxKernel("MaterialRealAux", name, params);
}

void NEMLCPAction::_add_vector_aux(std::string name, size_t index,
                                   std::string vname, std::string when)
{
  auto params = _factory.getValidParams("MaterialStdVectorAux");

  params.set<MaterialPropertyName>("property") = name;
  params.set<unsigned int>("index") = index;
  params.set<AuxVariableName>("variable") = vname;

  params.set<ExecFlagEnum>("execute_on") = when;

  _problem->addAuxKernel("MaterialStdVectorAux", "q"+std::to_string(index), 
                         params);
}

