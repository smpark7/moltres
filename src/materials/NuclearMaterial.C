#include "NuclearMaterial.h"
#include "MooseUtils.h"

#include <iomanip>
#include <sstream>
// #define PRINT(var) #var

registerMooseObject("MoltresApp", NuclearMaterial);

InputParameters
NuclearMaterial::validParams()
{
  InputParameters params = GenericConstantMaterial::validParams();
  params.addRequiredParam<unsigned int>("num_groups",
                                    "The number of groups the energy spectrum is divided into.");
  params.addRequiredParam<unsigned int>("num_precursor_groups",
                                    "The number of delayed neutron precursor groups.");
  params.addCoupledVar(
      "temperature", 937, "The temperature field for determining group constants.");
  params.addRequiredParam<MooseEnum>("interp_type",
                                     NuclearMaterial::interpTypes(),
                                     "The type of interpolation to perform.");
  params.addParam<bool>(
      "sss2_input", true, "Whether serpent 2 was used to generate the input files.");
  params.addParam<bool>("warn_out_of_range",
                        true,
                        "Warn the first time a state variable is sampled below or above its "
                        "tabulated range. Outside the range, linear interpolation holds the values "
                        "at the end of the range, and spline and monotone_cubic interpolation "
                        "evaluate the cubic of the end interval.");
  // Not named group_fluxes: inputs commonly set that in GlobalParams, and coupling it here would
  // make every nuclear material require the flux variables
  params.addCoupledVar("reaction_rate_fluxes",
                       "The group fluxes, by decreasing energy/increasing group number. If given, "
                       "the group-summed fission neutron rate (fission_neutron_rate), fission heat "
                       "rate (fission_heat_rate) and in-scatter rates (inscatter_rate) are computed "
                       "once per quadrature point, and CoupledFissionKernel, InScatter and "
                       "ElmIntegTotFissHeatPostprocessor use them instead of summing over the "
                       "groups themselves.");
  params.addParam<bool>("use_exp_form",
                        false,
                        "Whether the group fluxes are in an exponential/logarithmic format. Must match "
                        "the kernels' use_exp_form.");
  params.set<MooseEnum>("constant_on") = "NONE";
  // the following two lines esentially make the two parameters optional
  params.set<std::vector<std::string>>("prop_names") = std::vector<std::string>();
  params.set<std::vector<Real>>("prop_values") = std::vector<Real>();
  return params;
}

NuclearMaterial::NuclearMaterial(const InputParameters & parameters)
  : GenericConstantMaterial(parameters),
    _temperature(coupledValue("temperature")),
    _num_groups(getParam<unsigned int>("num_groups")),
    _num_precursor_groups(getParam<unsigned int>("num_precursor_groups")),
    _remxs(declareProperty<std::vector<Real>>("remxs")),
    _fissxs(declareProperty<std::vector<Real>>("fissxs")),
    _nsf(declareProperty<std::vector<Real>>("nsf")),
    _fisse(declareProperty<std::vector<Real>>("fisse")),
    _diffcoef(declareProperty<std::vector<Real>>("diffcoef")),
    _recipvel(declareProperty<std::vector<Real>>("recipvel")),
    _chi_t(declareProperty<std::vector<Real>>("chi_t")),
    _chi_p(declareProperty<std::vector<Real>>("chi_p")),
    _chi_d(declareProperty<std::vector<Real>>("chi_d")),
    _gtransfxs(declareProperty<std::vector<Real>>("gtransfxs")),
    _beta_eff(declareProperty<std::vector<Real>>("beta_eff")),
    _beta(declareProperty<Real>("beta")),
    _decay_constant(declareProperty<std::vector<Real>>("decay_constant")),

    _d_remxs_d_temp(declareProperty<std::vector<Real>>("d_remxs_d_temp")),
    _d_fissxs_d_temp(declareProperty<std::vector<Real>>("d_fissxs_d_temp")),
    _d_nsf_d_temp(declareProperty<std::vector<Real>>("d_nsf_d_temp")),
    _d_fisse_d_temp(declareProperty<std::vector<Real>>("d_fisse_d_temp")),
    _d_diffcoef_d_temp(declareProperty<std::vector<Real>>("d_diffcoef_d_temp")),
    _d_recipvel_d_temp(declareProperty<std::vector<Real>>("d_recipvel_d_temp")),
    _d_chi_t_d_temp(declareProperty<std::vector<Real>>("d_chi_t_d_temp")),
    _d_chi_p_d_temp(declareProperty<std::vector<Real>>("d_chi_p_d_temp")),
    _d_chi_d_d_temp(declareProperty<std::vector<Real>>("d_chi_d_d_temp")),
    _d_gtransfxs_d_temp(declareProperty<std::vector<Real>>("d_gtransfxs_d_temp")),
    _d_beta_eff_d_temp(declareProperty<std::vector<Real>>("d_beta_eff_d_temp")),
    _d_beta_d_temp(declareProperty<Real>("d_beta_d_temp")),
    _d_decay_constant_d_temp(declareProperty<std::vector<Real>>("d_decay_constant_d_temp")),
    _compute_reaction_rates(isCoupled("reaction_rate_fluxes")),
    _use_exp_form(getParam<bool>("use_exp_form")),
    _sss2_input(getParam<bool>("sss2_input")),
    _interp_type(getParam<MooseEnum>("interp_type"))
{
  if (_compute_reaction_rates)
  {
    if (coupledComponents("reaction_rate_fluxes") != _num_groups)
      paramError("reaction_rate_fluxes", "There must be one group flux variable per energy group.");
    _group_fluxes.resize(_num_groups);
    for (unsigned int i = 0; i < _num_groups; ++i)
      _group_fluxes[i] = &coupledValue("reaction_rate_fluxes", i);
    _conc.resize(_num_groups);
    _fission_neutron_rate = &declareProperty<Real>("fission_neutron_rate");
    _fission_heat_rate = &declareProperty<Real>("fission_heat_rate");
    _inscatter_rate = &declareProperty<std::vector<Real>>("inscatter_rate");
  }

  auto n = _xsec_names.size();
  for (decltype(n) j = 0; j < n; ++j)
  {
    if (_xsec_names[j].compare("GTRANSFXS") == 0)
      _vec_lengths[_xsec_names[j]] = _num_groups * _num_groups;
    else if (_xsec_names[j].compare("BETA_EFF") == 0 || _xsec_names[j].compare("DECAY_CONSTANT") == 0)
      _vec_lengths[_xsec_names[j]] = _num_precursor_groups;
    else
      _vec_lengths[_xsec_names[j]] = _num_groups;

    _xsec_values.push_back(&_xsec_map[_xsec_names[j]]);
    _spline_interps.push_back(&_xsec_spline_interpolators[_xsec_names[j]]);
    _monotone_cubic_interps.push_back(&_xsec_monotone_cubic_interpolators[_xsec_names[j]]);
    _linear_interps.push_back(&_xsec_linear_interpolators[_xsec_names[j]]);
  }
}

void
NuclearMaterial::buildXsTable(const std::vector<XsAxes> & grids,
                              const std::vector<std::string> & axis_names,
                              unsigned int own_axis,
                              unsigned int lsq_order)
{
  XsInterpolation type;
  switch (_interp_type)
  {
    case NONE:
      type = XsInterpolation::NONE;
      break;
    case LINEAR:
      type = XsInterpolation::LINEAR;
      break;
    case SPLINE:
      type = XsInterpolation::SPLINE;
      break;
    case MONOTONE_CUBIC:
      type = XsInterpolation::MONOTONE_CUBIC;
      break;
    case LSQ:
      type = XsInterpolation::LEAST_SQUARES;
      break;
    default:
      mooseError("interp_type '", _interp_type, "' is not supported by the group-constant table");
  }

  _xs_table = XsTable(type, lsq_order);
  _xs_offsets.resize(_xsec_names.size());
  for (unsigned int j = 0; j < _xsec_names.size(); ++j)
    _xs_offsets[j] = _xs_table.addComponents(grids[j], *_xsec_values[j], _xsec_names[j]);
  _xs_table.finalize();

  if (type == XsInterpolation::LEAST_SQUARES && _tid == 0)
  {
    std::ostringstream oss;
    oss << "Least-squares fit of order " << lsq_order
        << ": largest deviation from the tabulated values, relative to the largest magnitude of "
           "each group constant:";
    for (const auto & [name, residual] : _xs_table.residuals())
      if (_vec_lengths[name] > 0)
        oss << "\n  " << std::left << std::setw(15) << name << " " << std::scientific
            << std::setprecision(2) << residual;
    mooseInfo(oss.str());
  }

  _xs_state.assign(axis_names.size(), 0.);
  _xs_own_axis = own_axis;
  _xs_axis_names = axis_names;
  _xs_warn = getParam<bool>("warn_out_of_range") && type != XsInterpolation::NONE &&
             _xs_table.dimension() == axis_names.size();
  _xs_warned_low.assign(axis_names.size(), false);
  _xs_warned_high.assign(axis_names.size(), false);
}

void
NuclearMaterial::computeXsState()
{
  _xs_state[0] = _temperature[_qp];
}

void
NuclearMaterial::residualSetup()
{
  GenericConstantMaterial::residualSetup();
  // Every process holds every material, so all of them take part in the reductions
  if (!_xs_warn)
    return;

  auto lo = _xs_table.sampledMin();
  auto hi = _xs_table.sampledMax();
  _communicator.min(lo);
  _communicator.max(hi);

  const auto rule = _interp_type == SPLINE || _interp_type == MONOTONE_CUBIC
                        ? "extrapolated with the cubic of the end interval"
                        : "held at the values at the end of the range";
  for (unsigned int a = 0; a < _xs_axis_names.size(); ++a)
  {
    const Real lower = _xs_table.lowerBound(a);
    const Real upper = _xs_table.upperBound(a);
    // An axis with a single knot is constant by design
    if (lower == upper)
      continue;
    if (!_xs_warned_low[a] && lo[a] < lower)
    {
      _xs_warned_low[a] = true;
      if (processor_id() == 0)
        mooseWarning(_xs_axis_names[a], " was sampled at ", lo[a], ", below the tabulated range [",
                     lower, ", ", upper, "]; the group constants are ", rule,
                     ". Further sampling below the range is not reported.");
    }
    if (!_xs_warned_high[a] && hi[a] > upper)
    {
      _xs_warned_high[a] = true;
      if (processor_id() == 0)
        mooseWarning(_xs_axis_names[a], " was sampled at ", hi[a], ", above the tabulated range [",
                     lower, ", ", upper, "]; the group constants are ", rule,
                     ". Further sampling above the range is not reported.");
    }
  }
}

void
NuclearMaterial::computeProperties()
{
  GenericConstantMaterial::computeProperties();
  if (_compute_reaction_rates)
    for (_qp = 0; _qp < _qrule->n_points(); ++_qp)
      computeQpReactionRates();
}

void
NuclearMaterial::computePropertiesAtQp(unsigned int qp)
{
  GenericConstantMaterial::computePropertiesAtQp(qp);
  if (_compute_reaction_rates)
    computeQpReactionRates();
}

void
NuclearMaterial::computeQpReactionRates()
{
  for (unsigned int i = 0; i < _num_groups; ++i)
    _conc[i] = _use_exp_form ? std::exp((*_group_fluxes[i])[_qp]) : (*_group_fluxes[i])[_qp];

  Real fission = 0;
  Real heat = 0;
  for (unsigned int i = 0; i < _num_groups; ++i)
  {
    fission += _nsf[_qp][i] * _conc[i];
    heat += _fisse[_qp][i] * _fissxs[_qp][i] * _conc[i];
  }
  (*_fission_neutron_rate)[_qp] = fission;
  (*_fission_heat_rate)[_qp] = heat;

  // gtransfxs index of g' -> g follows InScatter
  auto & inscatter = (*_inscatter_rate)[_qp];
  inscatter.resize(_num_groups);
  const auto & gtransfxs = _gtransfxs[_qp];
  for (unsigned int g = 0; g < _num_groups; ++g)
  {
    Real r = 0;
    for (unsigned int i = 0; i < _num_groups; ++i)
      if (i != g)
        r += gtransfxs[_sss2_input ? i * _num_groups + g : i + g * _num_groups] * _conc[i];
    inscatter[g] = r;
  }
}

void
NuclearMaterial::dummyComputeQpProperties()
{
  tableComputeQpProperties();
}

void
NuclearMaterial::splineComputeQpProperties()
{
  tableComputeQpProperties();
}

void
NuclearMaterial::monotoneCubicComputeQpProperties()
{
  tableComputeQpProperties();
}

void
NuclearMaterial::linearComputeQpProperties()
{
  tableComputeQpProperties();
}

void
NuclearMaterial::tableComputeQpProperties()
{
  // Only Jacobian assembly reads the temperature derivatives
  const bool derivatives = _fe_problem.currentlyComputingJacobian();
  computeXsState();
  _xs_table.evaluate(_xs_state, _xs_own_axis, derivatives);

  const auto copy = [this](MaterialProperty<std::vector<Real>> & prop, XsIndex j, const Real * src)
  { std::copy_n(src + _xs_offsets[j], prop[_qp].size(), prop[_qp].begin()); };

  const Real * v = _xs_table.values().data();
  copy(_remxs, REMXS, v);
  copy(_fissxs, FISSXS, v);
  copy(_nsf, NSF, v);
  for (unsigned int i = 0; i < _num_groups; ++i)
    _fisse[_qp][i] = v[_xs_offsets[FISSE] + i] * 1e6 * 1.6e-19; // convert from MeV to Joules
  copy(_diffcoef, DIFFCOEF, v);
  copy(_recipvel, RECIPVEL, v);
  copy(_chi_t, CHI_T, v);
  copy(_chi_p, CHI_P, v);
  copy(_chi_d, CHI_D, v);
  copy(_gtransfxs, GTRANSFXS, v);
  copy(_beta_eff, BETA_EFF, v);
  copy(_decay_constant, DECAY_CONSTANT, v);
  _beta[_qp] = 0;
  for (unsigned int i = 0; i < _num_precursor_groups; ++i)
    _beta[_qp] += _beta_eff[_qp][i];

  if (!derivatives)
    return;

  const Real * dv = _xs_table.derivatives().data();
  copy(_d_remxs_d_temp, REMXS, dv);
  copy(_d_fissxs_d_temp, FISSXS, dv);
  copy(_d_nsf_d_temp, NSF, dv);
  for (unsigned int i = 0; i < _num_groups; ++i)
    _d_fisse_d_temp[_qp][i] = dv[_xs_offsets[FISSE] + i] * 1e6 * 1.6e-19;
  copy(_d_diffcoef_d_temp, DIFFCOEF, dv);
  copy(_d_recipvel_d_temp, RECIPVEL, dv);
  copy(_d_chi_t_d_temp, CHI_T, dv);
  copy(_d_chi_p_d_temp, CHI_P, dv);
  copy(_d_chi_d_d_temp, CHI_D, dv);
  copy(_d_gtransfxs_d_temp, GTRANSFXS, dv);
  copy(_d_beta_eff_d_temp, BETA_EFF, dv);
  copy(_d_decay_constant_d_temp, DECAY_CONSTANT, dv);
  _d_beta_d_temp[_qp] = 0;
  for (unsigned int i = 0; i < _num_precursor_groups; ++i)
    _d_beta_d_temp[_qp] += _d_beta_eff_d_temp[_qp][i];
}

void
NuclearMaterial::preComputeQpProperties()
{
  for (unsigned int i = 0; i < _num_props; i++)
    (*_properties[i])[_qp] = _prop_values[i];
  _remxs[_qp].resize(_num_groups);
  _fissxs[_qp].resize(_num_groups);
  _nsf[_qp].resize(_num_groups);
  _fisse[_qp].resize(_num_groups);
  _diffcoef[_qp].resize(_num_groups);
  _recipvel[_qp].resize(_num_groups);
  _chi_t[_qp].resize(_num_groups);
  _chi_p[_qp].resize(_num_groups);
  _chi_d[_qp].resize(_num_groups);

  _gtransfxs[_qp].resize(_num_groups * _num_groups);

  _beta_eff[_qp].resize(_num_precursor_groups);
  _decay_constant[_qp].resize(_num_precursor_groups);

  _d_remxs_d_temp[_qp].resize(_num_groups);
  _d_fissxs_d_temp[_qp].resize(_num_groups);
  _d_nsf_d_temp[_qp].resize(_num_groups);
  _d_fisse_d_temp[_qp].resize(_num_groups);
  _d_diffcoef_d_temp[_qp].resize(_num_groups);
  _d_recipvel_d_temp[_qp].resize(_num_groups);
  _d_chi_t_d_temp[_qp].resize(_num_groups);
  _d_chi_p_d_temp[_qp].resize(_num_groups);
  _d_chi_d_d_temp[_qp].resize(_num_groups);

  _d_gtransfxs_d_temp[_qp].resize(_num_groups * _num_groups);

  _d_beta_eff_d_temp[_qp].resize(_num_precursor_groups);
  _d_decay_constant_d_temp[_qp].resize(_num_precursor_groups);
}
