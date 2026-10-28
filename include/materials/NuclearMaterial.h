#pragma once

#include "GenericConstantMaterial.h"
#include "SplineInterpolation.h"
#include "BicubicSplineInterpolation.h"
#include "MonotoneCubicInterpolation.h"
#include "LinearInterpolation.h"
#include "XsTable.h"
#include "json.h"

/**
 * This class hinges on the `interp_type` chosen by the user. The user can choose from the
 * following interpolation types:
 *
 * "normal interpolations"
 * bicubic_spline : used when cross section is a function of two different temperatures (say fuel
 *                  and moderator). Performs bicubic spline interpolation. Example input property
 *                  table: property_file_dir/msr2g_Th_U_two_mat_homogenization_fuel_interp_NSF.txt
 * spline :         cubic spline interpolation for single temperature. Example input property
 *                  table: property_file_dir/newt_msre_fuel_NSF.txt
 * monotone_cubic : monotone cubic interpolation for single temperature. Same input property table
 *                  structure as spline
 * linear :         linear interpolation for single temperature. Same input property table
 *                  structure as spline
 *
 * "special interpolations"
 * none :           no interpolation is done. The cross section value for each group is read from
 *                  the first temperature row in the interpolation table
 * least_squares :  user should have performed a linear least squares fit on cross section data
 *                  and should supply interpolation table with each row corresponding to energy
 *                  group. Each row should have two columns corresponding to 'a' and 'b' where
 *                  xsec(T) = a * T + b
 */
class NuclearMaterial : public GenericConstantMaterial
{
public:
  NuclearMaterial(const InputParameters & parameters);

  static InputParameters validParams();

  // correspond to descriptions above
  enum INTERPOLATOR
  {
    BICUBIC,
    SPLINE,
    MONOTONE_CUBIC,
    LINEAR,
    NONE,
    LSQ
  };

  // returns a MooseEnum corresponding to the above enum
  static MooseEnum interpTypes()
  {
    return MooseEnum("bicubic=0 spline=1"
                     " monotone_cubic=2 linear=3 none=4 least_squares=5");
  }

protected:
  virtual void dummyComputeQpProperties();
  virtual void splineComputeQpProperties();
  virtual void monotoneCubicComputeQpProperties();
  virtual void linearComputeQpProperties();
  virtual void preComputeQpProperties();
  /// Sets every group-constant property from _xs_table at the current qp
  void tableComputeQpProperties();
  /**
   * Builds _xs_table from _xsec_map.
   * @param grids grids[j] holds the knots of each axis of _xsec_names[j]
   * @param axis_names Name of each axis, used in messages
   * @param own_axis Axis set by the temperature variable; the derivative properties are taken
   *                 along it. An index past the last axis means none (zero derivatives).
   * @param lsq_order Total degree of the polynomial for interp_type least_squares
   */
  void buildXsTable(const std::vector<XsAxes> & grids,
                    const std::vector<std::string> & axis_names,
                    unsigned int own_axis,
                    unsigned int lsq_order = 2);
  /// Sets _xs_state, the state point of the table at the current qp
  virtual void computeXsState();

  /// Reports the first sampling outside the tabulated range of each axis
  virtual void residualSetup() override;

  const VariableValue & _temperature;

  // Number of neutron groups
  unsigned int _num_groups;

  // Number of precursor groups
  unsigned int _num_precursor_groups;

  // Group constant MaterialProperty(s)
  MaterialProperty<std::vector<Real>> & _remxs;
  MaterialProperty<std::vector<Real>> & _fissxs;
  MaterialProperty<std::vector<Real>> & _nsf;
  MaterialProperty<std::vector<Real>> & _fisse;
  MaterialProperty<std::vector<Real>> & _diffcoef;
  MaterialProperty<std::vector<Real>> & _recipvel;
  MaterialProperty<std::vector<Real>> & _chi_t;
  MaterialProperty<std::vector<Real>> & _chi_p;
  MaterialProperty<std::vector<Real>> & _chi_d;
  MaterialProperty<std::vector<Real>> & _gtransfxs;
  MaterialProperty<std::vector<Real>> & _beta_eff;
  MaterialProperty<Real> & _beta;
  MaterialProperty<std::vector<Real>> & _decay_constant;
  MaterialProperty<std::vector<Real>> & _d_remxs_d_temp;
  MaterialProperty<std::vector<Real>> & _d_fissxs_d_temp;
  MaterialProperty<std::vector<Real>> & _d_nsf_d_temp;
  MaterialProperty<std::vector<Real>> & _d_fisse_d_temp;
  MaterialProperty<std::vector<Real>> & _d_diffcoef_d_temp;
  MaterialProperty<std::vector<Real>> & _d_recipvel_d_temp;
  MaterialProperty<std::vector<Real>> & _d_chi_t_d_temp;
  MaterialProperty<std::vector<Real>> & _d_chi_p_d_temp;
  MaterialProperty<std::vector<Real>> & _d_chi_d_d_temp;
  MaterialProperty<std::vector<Real>> & _d_gtransfxs_d_temp;
  MaterialProperty<std::vector<Real>> & _d_beta_eff_d_temp;
  MaterialProperty<Real> & _d_beta_d_temp;
  MaterialProperty<std::vector<Real>> & _d_decay_constant_d_temp;

  // Group constant interpolation type
  MooseEnum _interp_type;

  // Vector of group constant names
  std::vector<std::string> _xsec_names{"REMXS",
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
                                       "DECAY_CONSTANT"};

  // Map of group constant names to group constant values
  std::map<std::string, std::vector<std::vector<Real>>> _xsec_map;

  // Group constant interpolators
  std::map<std::string, std::vector<SplineInterpolation>> _xsec_spline_interpolators;
  std::map<std::string, std::vector<MonotoneCubicInterpolation>> _xsec_monotone_cubic_interpolators;
  std::map<std::string, std::vector<LinearInterpolation>> _xsec_linear_interpolators;
  std::map<std::string, std::vector<BicubicSplineInterpolation>> _xsec_bicubic_spline_interpolators;

  // Indices into the containers below; must follow the order of _xsec_names
  enum XsIndex
  {
    REMXS,
    FISSXS,
    NSF,
    FISSE,
    DIFFCOEF,
    RECIPVEL,
    CHI_T,
    CHI_P,
    CHI_D,
    GTRANSFXS,
    BETA_EFF,
    DECAY_CONSTANT
  };

  // Pointers to the map entries above, resolved once in the constructor so the per-qp
  // property loops avoid string-keyed map lookups. std::map never relocates its values, so
  // these stay valid while derived classes fill the containers.
  std::vector<std::vector<std::vector<Real>> *> _xsec_values;
  std::vector<std::vector<SplineInterpolation> *> _spline_interps;
  std::vector<std::vector<MonotoneCubicInterpolation> *> _monotone_cubic_interps;
  std::vector<std::vector<LinearInterpolation> *> _linear_interps;

  // Map of group constant names to number of neutron/precursor groups
  std::map<std::string, int> _vec_lengths;

  // Vector of temperature values
  std::vector<double> _XsTemperature;

  /// All group constants, interpolated together at each qp
  XsTable _xs_table;
  /// Offset of each XsIndex entry in the flat component list of _xs_table
  std::vector<unsigned int> _xs_offsets;
  /// State point of _xs_table at the current qp, one coordinate per axis
  std::vector<Real> _xs_state;
  /// Axis of _xs_table set by the temperature variable
  unsigned int _xs_own_axis = 0;
  /// Name of each axis of _xs_table
  std::vector<std::string> _xs_axis_names;
  /// Whether sampling outside the table range is reported
  bool _xs_warn = false;
  /// Whether sampling below/above the range of each axis has been reported
  std::vector<bool> _xs_warned_low;
  std::vector<bool> _xs_warned_high;

  std::vector<std::vector<Real>> _remxs_consts = std::vector<std::vector<Real>>(2);
  std::vector<std::vector<Real>> _fissxs_consts = std::vector<std::vector<Real>>(2);
  std::vector<std::vector<Real>> _nubar_consts = std::vector<std::vector<Real>>(2);
  std::vector<std::vector<Real>> _nsf_consts = std::vector<std::vector<Real>>(2);
  std::vector<std::vector<Real>> _fisse_consts = std::vector<std::vector<Real>>(2);
  std::vector<std::vector<Real>> _diffcoeff_consts = std::vector<std::vector<Real>>(2);
  std::vector<std::vector<Real>> _recipvel_consts = std::vector<std::vector<Real>>(2);
  std::vector<std::vector<Real>> _chi_t_consts = std::vector<std::vector<Real>>(2);
  std::vector<std::vector<Real>> _chi_p_consts = std::vector<std::vector<Real>>(2);
  std::vector<std::vector<Real>> _chi_d_consts = std::vector<std::vector<Real>>(2);
  std::vector<std::vector<Real>> _gtransfxs_consts = std::vector<std::vector<Real>>(2);
  std::vector<std::vector<Real>> _beta_eff_consts = std::vector<std::vector<Real>>(2);
  std::vector<std::vector<Real>> _decay_constants_consts = std::vector<std::vector<Real>>(2);
};
