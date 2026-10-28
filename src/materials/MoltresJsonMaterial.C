#include "MoltresJsonMaterial.h"
#include "MooseUtils.h"
// #define PRINT(var) #var

registerMooseObject("MoltresApp", MoltresJsonMaterial);

InputParameters
MoltresJsonMaterial::validParams()
{
  InputParameters params = NuclearMaterial::validParams();
  params.addRequiredParam<std::string>("base_file", "The file containing macroscopic XS.");
  params.addRequiredParam<std::string>("material_key",
                                       "The file key where the macroscopic XS can be found.");
  params.addParam<std::vector<std::string>>(
      "group_constants",
      std::vector<std::string>{"REMXS",
                               "FISSXS",
                               "NSF",
                               "FISSE",
                               "DIFFCOEF",
                               "RECIPVEL",
                               "CHI_T",
                               "CHI_P",
                               "CHI_D",
                               "GTRANSFXS",
                               "BETA_EFF",
                               "DECAY_CONSTANT"},
      "Group constants to be determined.");
  params.addParam<MooseEnum>(
      "material_type",
      MooseEnum("fuel moderator none"),
      "Which state variable of a multi-dimensional table (one with 'axes') the 'temperature' "
      "variable is: fuel_temperature (fuel), moderator_temperature (moderator) or neither (none). "
      "Required for, and only accepted with, such tables.");
  params.addParam<MooseFunctorName>(
      "fuel_temperature",
      "Fuel temperature for the fuel_temperature axis of a multi-dimensional table, if "
      "'material_type' is not fuel. Any functor: a variable, function, constant or spatial user "
      "object.");
  params.addParam<MooseFunctorName>(
      "moderator_temperature",
      "Moderator temperature for the moderator_temperature axis of a multi-dimensional table, if "
      "'material_type' is not moderator. Any functor: a variable, function, constant or spatial "
      "user object.");
  params.addParamNamesToGroup("material_type fuel_temperature moderator_temperature",
                              "Multi-dimensional tables");
  params.addRangeCheckedParam<unsigned int>(
      "lsq_order",
      2,
      "lsq_order <= 4",
      "Total degree of the polynomial fitted to the tabulated group constants with "
      "interp_type = least_squares (cross terms included). Every state variable with more than "
      "one knot needs at least lsq_order + 1 knots.");
  return params;
}

MoltresJsonMaterial::MoltresJsonMaterial(const InputParameters & parameters)
  : NuclearMaterial(parameters),
    _group_consts(getParam<std::vector<std::string>>("group_constants")),
    _material_key(getParam<std::string>("material_key"))
{
  std::string base_file = getParam<std::string>("base_file");

  const std::string & file_name_ref = base_file;
  std::ifstream myfile(file_name_ref.c_str());
  if (!myfile.good())
    mooseError("Unable to open XS file: " + base_file);
  nlohmann::json xs_root;
  myfile >> xs_root;

  if (isParamSetByUser("lsq_order") && _interp_type != LSQ)
    paramError("lsq_order", "Only interp_type = least_squares takes this parameter.");

  if (xs_root[_material_key].contains("axes"))
  {
    constructMultiD(xs_root[_material_key]);
    return;
  }

  for (const auto & param : {"material_type", "fuel_temperature", "moderator_temperature"})
    if (isParamSetByUser(param))
      paramError(param,
                 "Only multi-dimensional tables ('axes' in the material entry) take this "
                 "parameter; '",
                 _material_key,
                 "' is tabulated on 'temp' only.");

  int k = 0;
  auto temp_root = xs_root[_material_key]["temp"];
  _XsTemperature.resize(temp_root.size());

  for (auto & el : temp_root.items())
  //for (moosecontrib::Json::ValueIterator itr = temp_root.begin(); itr != temp_root.end(); itr++)
  {
    _XsTemperature[k] = el.value().get<int>();
    k = k + 1;
  }

  Construct(xs_root);
  checkLsqKnots({_XsTemperature}, {"temperature"});
  buildXsTable(std::vector<XsAxes>(_xsec_names.size(), {_XsTemperature}),
               {"temperature"},
               0,
               getParam<unsigned int>("lsq_order"));
}

void
MoltresJsonMaterial::constructMultiD(const nlohmann::json & material)
{
  // Supported state variables and the material type whose own temperature each one is
  const std::map<std::string, std::string> axis_types = {{"fuel_temperature", "fuel"},
                                                         {"moderator_temperature", "moderator"}};

  if (!isParamValid("material_type"))
    paramError("interp_type",
               "'",
               _material_key,
               "' is a multi-dimensional table; set 'material_type' to say which state variable "
               "the 'temperature' variable is.");
  const std::string material_type = getParam<MooseEnum>("material_type");

  const auto & axes_root = material["axes"];
  if (!axes_root.is_object() || axes_root.empty())
    mooseError("'axes' of '", _material_key, "' must map each state variable to its knots.");

  std::vector<std::string> axis_names;
  XsAxes axes;
  _axis_functors.clear();
  unsigned int own_axis = axes_root.size();
  for (const auto & [name, knots] : axes_root.items())
  {
    const auto it = axis_types.find(name);
    if (it == axis_types.end())
      mooseError("Unsupported state variable '", name, "' in 'axes' of '", _material_key,
                 "'. Supported: fuel_temperature, moderator_temperature.");
    if (!knots.is_array() || knots.empty())
      mooseError("'axes/", name, "' of '", _material_key, "' must be a non-empty list of knots.");

    axis_names.push_back(name);
    axes.push_back(knots.get<std::vector<Real>>());
    if (it->second == material_type)
    {
      if (isParamValid(name))
        paramError(name, "The material's own state variable comes from 'temperature' "
                   "(material_type = ",
                   material_type,
                   ").");
      own_axis = axis_names.size() - 1;
      _axis_functors.push_back(nullptr);
    }
    else
    {
      if (!isParamValid(name))
        paramError("material_type",
                   "'",
                   _material_key,
                   "' depends on ",
                   name,
                   "; with material_type = ",
                   material_type,
                   ", set the '",
                   name,
                   "' parameter.");
      _axis_functors.push_back(&getFunctor<Real>(name));
    }
  }
  for (const auto & [name, axis_type] : axis_types)
    if (isParamValid(name) && !axes_root.contains(name))
      paramError(name, "'", _material_key, "' does not depend on ", name, ".");

  // Place each branch at its knot on every axis (row-major, last axis fastest)
  std::size_t n_branches = 1;
  for (const auto & knots : axes)
    n_branches *= knots.size();
  std::vector<const nlohmann::json *> branches(n_branches, nullptr);
  std::vector<std::string> labels(n_branches);

  const auto & branch_root = material["branches"];
  if (!branch_root.is_array())
    mooseError("'", _material_key, "' has 'axes' but no 'branches' list.");
  for (std::size_t i = 0; i < branch_root.size(); ++i)
  {
    const auto & branch = branch_root[i];
    std::size_t index = 0;
    std::string state;
    for (const auto a : index_range(axes))
    {
      const auto & name = axis_names[a];
      if (!branch.contains(name) || !branch[name].is_number())
        mooseError("Branch ", i, " of '", _material_key, "' does not give ", name, ".");
      const Real x = branch[name].get<Real>();
      const auto & knots = axes[a];
      const auto knot = std::find_if(knots.begin(),
                                     knots.end(),
                                     [x](Real k) { return MooseUtils::absoluteFuzzyEqual(x, k); });
      if (knot == knots.end())
        mooseError("Branch ", i, " of '", _material_key, "' has ", name, " = ", x,
                   ", which is not in 'axes/", name, "'.");
      index = index * knots.size() + std::distance(knots.begin(), knot);
      state += (state.empty() ? "" : ", ") + name + " = " + Moose::stringify(*knot);
    }
    if (branches[index])
      mooseError("'", _material_key, "' has two branches at ", state, ".");
    branches[index] = &branch;
    labels[index] = _material_key + "/branches/" + std::to_string(i) + " (" + state + ")";
  }
  for (const auto b : index_range(branches))
    if (!branches[b])
    {
      std::string state;
      for (std::size_t a = axes.size(), r = b; a-- > 0; r /= axes[a].size())
        state = axis_names[a] + " = " + Moose::stringify(axes[a][r % axes[a].size()]) +
                (state.empty() ? "" : ", " + state);
      mooseError("'", _material_key, "' has no branch at ", state,
                 ". Multi-dimensional tables need a branch at every combination of knots.");
    }

  switch (_interp_type)
  {
    case NONE:
      if (n_branches > 1)
        mooseError("Group constant data for '" + _material_key + "' provided at "
                   "multiple state points with interp_type=none. Remove extra branches or "
                   "change interpolation scheme.");
      break;
    case LINEAR:
    case LSQ:
      break;
    case SPLINE:
    case MONOTONE_CUBIC:
      if (axes.size() > 1)
        paramError("interp_type",
                   "Spline and monotone cubic interpolation support tables with one state "
                   "variable only; use linear.");
      break;
    default:
      paramError("interp_type", "Not supported for multi-dimensional tables.");
  }

  readGroupConstants(branches, labels);
  checkLsqKnots(axes, axis_names);
  buildXsTable(std::vector<XsAxes>(_xsec_names.size(), axes),
               axis_names,
               own_axis,
               getParam<unsigned int>("lsq_order"));
}

void
MoltresJsonMaterial::checkLsqKnots(const XsAxes & axes, const std::vector<std::string> & axis_names)
{
  if (_interp_type != LSQ)
    return;
  const auto order = getParam<unsigned int>("lsq_order");
  for (const auto a : index_range(axes))
    if (axes[a].size() > 1 && axes[a].size() < order + 1)
      paramError("lsq_order",
                 "A fit of order ",
                 order,
                 " needs at least ",
                 order + 1,
                 " knots on every state variable with more than one; '",
                 _material_key,
                 "' has ",
                 axes[a].size(),
                 " on ",
                 axis_names[a],
                 ".");
}

void
MoltresJsonMaterial::readGroupConstants(const std::vector<const nlohmann::json *> & branches,
                                        const std::vector<std::string> & labels)
{
  std::set<std::string> gc_set(_group_consts.begin(), _group_consts.end());
  bool oneInfo = false;
  for (unsigned int j = 0; j < _xsec_names.size(); ++j)
  {
    auto o = _vec_lengths[_xsec_names[j]];
    _xsec_map[_xsec_names[j]].resize(o);

    if (gc_set.find(_xsec_names[j]) != gc_set.end())
    {
      for (const auto l : index_range(branches))
      {
        const auto & branch = *branches[l];
        const auto dataset =
            branch.contains(_xsec_names[j]) ? branch[_xsec_names[j]] : nlohmann::json();
        if (_xsec_names[j] == "CHI_D" && dataset.empty())
        {
          for (decltype(_num_groups) k = 1; k < _num_groups; ++k)
            _xsec_map["CHI_D"][k].push_back(0.0);
          _xsec_map["CHI_D"][0].push_back(1.0);
          mooseWarning(
              "CHI_D data missing -> assume delayed neutrons born in top group for material " +
              _name);
          continue;
        }
        if (dataset.empty())
          mooseError("Unable to open database " + labels[l] + "/" + _xsec_names[j]);

        int dims = dataset.size();
        if (o == 0 and !oneInfo)
        {
          mooseInfo("Only precursor material data initialized (num_groups = 0) for material " + _name);
          oneInfo = true;
        }
        if (o != dims && o != 0)
          mooseError("The number of " + labels[l] + "/" + _xsec_names[j] +
                     " values does not match the "
                     "num_groups/num_precursor_groups parameter. " +
                     std::to_string(dims) + "!=" + std::to_string(o));
        for (auto k = 0; k < o; ++k)
          _xsec_map[_xsec_names[j]][k].push_back(dataset[k].get<double>());
      }
    }
    else
    {
      for (const auto l : index_range(branches))
      {
        libmesh_ignore(l);
        for (auto k = 0; k < o; ++k)
          _xsec_map[_xsec_names[j]][k].push_back(0.);
      }
    }
  }
}

void
MoltresJsonMaterial::Construct(nlohmann::json xs_root)
{
  std::vector<const nlohmann::json *> branches;
  std::vector<std::string> labels;
  for (const auto temperature : _XsTemperature)
  {
    auto temp_key = std::to_string(static_cast<int>(temperature));
    branches.push_back(&xs_root[_material_key][temp_key]);
    labels.push_back(_material_key + "/" + temp_key);
  }
  readGroupConstants(branches, labels);

  for (unsigned int j = 0; j < _xsec_names.size(); ++j)
  {
    auto o = _vec_lengths[_xsec_names[j]];
    auto L = _XsTemperature.size();

    _xsec_linear_interpolators[_xsec_names[j]].resize(o);
    _xsec_spline_interpolators[_xsec_names[j]].resize(o);
    _xsec_monotone_cubic_interpolators[_xsec_names[j]].resize(o);

    switch (_interp_type)
    {
      case LSQ:
        break;
      case BICUBIC:
        mooseError("BICUBIC not supported, please select \
          NONE, LINEAR, SPLINE, MONOTONE_CUBIC, or LEAST_SQUARES ");
      case NONE:
        if (L > 1)
            mooseError(
                "Group constant data for '" + _material_key + "' provided at "
                "multiple temperatures with interp_type=none. Remove extra temperature data or "
                "change interpolation scheme.");
        break;
      case LINEAR:
        for (auto k = 0; k < o; ++k)
        {
          _xsec_linear_interpolators[_xsec_names[j]][k].setData(_XsTemperature,
                                                                _xsec_map[_xsec_names[j]][k]);
        }
        break;
      case SPLINE:
        for (auto k = 0; k < o; ++k)
        {
          _xsec_spline_interpolators[_xsec_names[j]][k].setData(_XsTemperature,
                                                                _xsec_map[_xsec_names[j]][k]);
        }
        break;
      case MONOTONE_CUBIC:
        if (L < 3)
          mooseError("Monotone cubic interpolation requires at least three data points.");
        for (auto k = 0; k < o; ++k)
        {
          _xsec_monotone_cubic_interpolators[_xsec_names[j]][k].setData(
              _XsTemperature,
              _xsec_map[_xsec_names[j]][k]);
        }
        break;
      default:
        mooseError("Invalid enum type for interp_type");
        break;
    }
  }
}

void
MoltresJsonMaterial::computeQpProperties()
{
  NuclearMaterial::preComputeQpProperties();

  switch (_interp_type)
  {
    case LSQ:
      tableComputeQpProperties();
      break;
    case BICUBIC:
      mooseError("BICUBIC not supported, please select \
                  NONE, LINEAR, SPLINE, MONOTONE_CUBIC, or LEAST_SQUARES ");
    case NONE:
      dummyComputeQpProperties();
      break;
    case LINEAR:
      linearComputeQpProperties();
      break;
    case SPLINE:
      splineComputeQpProperties();
      break;
    case MONOTONE_CUBIC:
      monotoneCubicComputeQpProperties();
      break;
    default:
      mooseError("Invalid enum type for interp_type");
      break;
  }
}

void
MoltresJsonMaterial::computeXsState()
{
  if (_axis_functors.empty())
  {
    NuclearMaterial::computeXsState();
    return;
  }

  const auto state = determineState();
  for (const auto a : index_range(_axis_functors))
  {
    const auto * functor = _axis_functors[a];
    if (!functor)
      _xs_state[a] = _temperature[_qp];
    else if (_bnd)
      _xs_state[a] = (*functor)(
          Moose::ElemSideQpArg{_current_elem, _current_side, _qp, _qrule, _q_point[_qp]}, state);
    else
    {
      const Elem * elem = _neighbor ? _current_elem->neighbor_ptr(_current_side) : _current_elem;
      _xs_state[a] = (*functor)(Moose::ElemQpArg{elem, _qp, _qrule, _q_point[_qp]}, state);
    }
  }
}
