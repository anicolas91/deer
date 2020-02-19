#include "AddVariableAction.h"
#include "CZMRVEStrainAction.h"
#include "FEProblem.h"
#include "RankTwoScalarTools.h"
#include "libmesh/string_to_enum.h"

registerMooseAction("DeerApp", CZMRVEStrainAction, "add_material");
registerMooseAction("DeerApp", CZMRVEStrainAction, "add_postprocessor");

const std::map<std::pair<int, int>, std::string> _tensor_map = {
    {std::make_pair(0, 0), "xx"}, {std::make_pair(1, 1), "yy"},
    {std::make_pair(2, 2), "zz"}, {std::make_pair(0, 1), "xy"},
    {std::make_pair(0, 2), "xz"}, {std::make_pair(1, 2), "yz"}};

const std::vector<std::string> interface_strain_tensors = {
    "interface_strain",
    "interface_opening_strain",
    "interface_sliding_strain",
    "interface_strain_rate",
    "interface_opening_strain_rate",
    "interface_sliding_strain_rate"};

const std::vector<std::string> bulk_strain_tensors = {
    "mechanical_strain", "inelastic_strain", "elastic_strain"};

InputParameters CZMRVEStrainAction::validParams() {
  InputParameters params = Action::validParams();
  params.addRequiredParam<std::vector<VariableName>>(
      "displacements", "The displacement variables");
  params.addRequiredParam<std::vector<BoundaryName>>(
      "boundary", "The list of boundary names  where the cohesive zone lives");
  params.addRequiredParam<std::vector<SubdomainName>>(
      "grains_block_names",
      "The list of boundary names  where the cohesive zone lives");
  return params;
}

CZMRVEStrainAction::CZMRVEStrainAction(const InputParameters &params)
    : Action(params),
      _displacements(getParam<std::vector<VariableName>>("displacements")),
      _n_disp(_displacements.size()),
      _boundary_names(getParam<std::vector<BoundaryName>>("boundary")),
      _grains_block_names(
          getParam<std::vector<SubdomainName>>("grains_block_names")) {}

void CZMRVEStrainAction::act() {

  if (_current_task == "add_material") {
    /**addinterface strainrate material*/
    auto params_mat = _factory.getValidParams("CZMStrainRate");
    params_mat.set<std::vector<VariableName>>("displacements") = _displacements;
    params_mat.set<std::vector<BoundaryName>>("boundary") = _boundary_names;
    _problem->addMaterial("CZMStrainRate", "interface_strain_rates",
                          params_mat);

    for (auto sname : bulk_strain_tensors) {
      /**addinterface bulk strainrate material*/
      auto params_mat = _factory.getValidParams("TensorRateMaterial");
      params_mat.set<MaterialPropertyName>("rank_two_tensor") = sname;
      params_mat.set<std::vector<SubdomainName>>("block") = _grains_block_names;
      _problem->addMaterial("TensorRateMaterial",
                            std::string("TensorRateMaterial") + "_" + sname,
                            params_mat);
    }

  } else if (_current_task == "add_postprocessor") {

    /* add volume pps*/
    /* start adding bulk volume*/
    auto params_vbulk = _factory.getValidParams("VolumePostprocessor");
    params_vbulk.set<ExecFlagEnum>("execute_on") = EXEC_INITIAL;
    params_vbulk.set<std::vector<SubdomainName>>("block") = _grains_block_names;
    _problem->addPostprocessor("VolumePostprocessor", "grain_volume",
                               params_vbulk);

    /* adding interface volume*/
    auto params_vinterface =
        _factory.getValidParams("InterfaceAreaPostprocessor");
    params_vinterface.set<ExecFlagEnum>("execute_on") = EXEC_INITIAL;
    params_vinterface.set<std::vector<BoundaryName>>("boundary") =
        _boundary_names;
    _problem->addPostprocessor("InterfaceAreaPostprocessor", "GB_area",
                               params_vinterface);

    for (auto tensor : interface_strain_tensors)
      addTensorPostProcessorInterface(tensor);
    for (auto tensor : bulk_strain_tensors) {
      addTensorPostProcessorBulk(tensor);
      addTensorPostProcessorBulk(tensor + "_rate");
    }
    /* compute RVEStrain average */
    sumStrain("strain");
    sumStrain("strain_rate");
  }
}

void CZMRVEStrainAction::addTensorPostProcessorBulk(const std::string mp_name) {
  for (auto entry : _tensor_map) {
    if ((entry.first.first < _n_disp) & (entry.first.second < _n_disp)) {
      auto params_pp = _factory.getValidParams("MaterialTensorAverage");

      params_pp.set<MaterialPropertyName>("rank_two_tensor") = mp_name;
      params_pp.set<unsigned int>("index_i") = entry.first.first;
      params_pp.set<unsigned int>("index_j") = entry.first.second;
      std::vector<PostprocessorName> ppnames = {"grain_volume"};
      params_pp.set<std::vector<PostprocessorName>>("volumes_PP") = ppnames;
      params_pp.set<ExecFlagEnum>("execute_on") = EXEC_TIMESTEP_END;

      params_pp.set<std::vector<SubdomainName>>("block") = _grains_block_names;
      _problem->addPostprocessor("MaterialTensorAverage",
                                 "grain_" + mp_name + "_" + entry.second,
                                 params_pp);
    }
  }
}

void CZMRVEStrainAction::addTensorPostProcessorInterface(
    const std::string mp_name) {
  for (auto entry : _tensor_map) {
    if ((entry.first.first < _n_disp) & (entry.first.second < _n_disp)) {
      auto params_pp =
          _factory.getValidParams("MaterialTensorAverageInterface");

      params_pp.set<MaterialPropertyName>("rank_two_tensor") = mp_name;
      params_pp.set<unsigned int>("index_i") = entry.first.first;
      params_pp.set<unsigned int>("index_j") = entry.first.second;
      params_pp.set<std::vector<BoundaryName>>("boundary") = _boundary_names;
      std::vector<PostprocessorName> ppnames = {"grain_volume"};
      params_pp.set<std::vector<PostprocessorName>>("volumes_PP") = ppnames;
      params_pp.set<ExecFlagEnum>("execute_on") = EXEC_TIMESTEP_END;
      _problem->addPostprocessor(
          "MaterialTensorAverageInterface",
          "GB_" + mp_name.substr(10) + "_" + entry.second, params_pp);
    }
  }
}

void CZMRVEStrainAction::sumStrain(const std::string mp_name) {
  for (auto entry : _tensor_map) {
    if ((entry.first.first < _n_disp) & (entry.first.second < _n_disp)) {
      auto params_pp = _factory.getValidParams("SumPostprocessor");

      params_pp.set<PostprocessorName>("value1") =
          "grain_mechanical_" + mp_name + "_" + entry.second;
      params_pp.set<PostprocessorName>("value2") =
          "GB_" + mp_name + "_" + entry.second;

      _problem->addPostprocessor(
          "SumPostprocessor", "RVE_" + mp_name + "_" + entry.second, params_pp);
    }
  }
}
