#include "SNScattering.h"

#include "MoltresUtils.h"
using MooseUtils::relativeFuzzyEqual;

registerMooseObject("MoltresApp", SNScattering);

InputParameters
SNScattering::validParams()
{
  InputParameters params = ArrayKernel::validParams();
  params.addRequiredParam<unsigned int>("N", "Discrete ordinate order");
  params.addRequiredParam<unsigned int>("group_number", "The current energy group");
  params.addRequiredParam<unsigned int>("num_groups", "The total number of energy groups");
  params.addParam<int>("L", 2, "Maximum scattering moment");
  params.addRequiredCoupledVar("group_angular_fluxes",
      "All the variables that hold the group angular fluxes. These MUST be listed by decreasing "
      "energy/increasing group number.");
  params.addCoupledVar("group_fluxes",
      "All the variables that hold the group fluxes. These MUST be listed by decreasing "
      "energy/increasing group number.");
  params.addParam<bool>("acceleration", false, "Whether an acceleration scheme is applied.");
  params.addParam<bool>("use_initial_flux", false,
      "Whether to use the diffusion flux as an initial condition");
  params.addParam<PostprocessorName>("iteration_postprocessor", 0,
      "Name of postprocessor with fixed point iteration number");
  return params;
}

SNScattering::SNScattering(const InputParameters & parameters)
  : ArrayKernel(parameters),
    _tau_sn(getMaterialProperty<std::vector<Real>>("tau_sn")),
    _scatter(getMaterialProperty<std::vector<Real>>("scatter")),
    _group(getParam<unsigned int>("group_number") - 1),
    _num_groups(getParam<unsigned int>("num_groups")),
    _L(getParam<int>("L")),
    _acceleration(getParam<bool>("acceleration")),
    _use_initial_flux(getParam<bool>("use_initial_flux")),
    _iteration_postprocessor(getPostprocessorValue("iteration_postprocessor")),
    _lhs(_count),
    _res_rhs(_count),
    _jac_rhs(_count)
{
  if (_acceleration || _use_initial_flux)
  {
    unsigned int n = coupledComponents("group_fluxes");
    if (n != _num_groups)
      mooseError("The number of group flux variables does not match the number of groups.");
    _group_fluxes.resize(n);
    for (unsigned int g = 0; g < _num_groups; ++g)
      _group_fluxes[g] = &coupledValue("group_fluxes", g);
  }

  unsigned int n = coupledComponents("group_angular_fluxes");
  if (n != _num_groups)
    mooseError("The number of angular group flux variables does not match the number of groups.");
  _group_angular_fluxes.resize(n);
  _flux_ids.resize(n);
  for (unsigned int g = 0; g < _num_groups; ++g)
  {
    _group_angular_fluxes[g] = &coupledArrayValue("group_angular_fluxes", g);
    _flux_ids[g] = coupled("group_angular_fluxes", g);
  }

  // Level-symmetric quadrature points, weights, and harmonics
  RealEigenMatrix ords_weights = MoltresUtils::level_symmetric(getParam<unsigned int>("N"));
  _ordinates = ords_weights.leftCols(3);
  _weights = ords_weights.col(3);
  _harmonics = MoltresUtils::sph_harmonics_mat(_L, _ordinates);
}

void
SNScattering::computeQpResidual(RealEigenVector & residual)
{
  // weight * lhs * rhs
  residual = _weights.cwiseProduct(
      _tau_sn[_qp][_group] * _ordinates * _array_grad_test[_i][_qp] +
      RealEigenVector::Constant(_count, _test[_i][_qp])
      ).cwiseProduct(_res_rhs);
}

void
SNScattering::initQpResidual()
{
  _res_rhs.setZero();
  // If using diffusion flux as initial condition, no need to compute scalar flux from
  // angular fluxes on the first iteration.
  int start_idx = 0;
  if (_acceleration || (_use_initial_flux && relativeFuzzyEqual(_iteration_postprocessor, 1.0)))
  {
    ++start_idx;
    for (unsigned int g = 0; g < _num_groups; ++g)
    {
      int scatter_idx = g * _num_groups + _group;
      if (_scatter[_qp][scatter_idx] == 0.0)
        continue;
      _res_rhs -= RealEigenVector::Constant(
          _count,
          _scatter[_qp][scatter_idx] * (*_group_fluxes[g])[_qp]) * _ls_norm_factor;
    }
  }

  // Compute scattering contributions up to order L
  Real flux_moment;
  int harmonics_idx = start_idx;
  for (int l = start_idx; l < _L+1; ++l)
  {
    for (int m = -l; m < l+1; ++m)
    {
      for (unsigned int g = 0; g < _num_groups; ++g)
      {
        int scatter_idx = l * _num_groups * _num_groups + g * _num_groups + _group;
        if (_scatter[_qp][scatter_idx] == 0.0)
          continue;
        flux_moment =
          _weights.cwiseProduct(_harmonics.col(harmonics_idx)).transpose() *
          (*_group_angular_fluxes[g])[_qp];
        _res_rhs -=
          _scatter[_qp][scatter_idx] * flux_moment *
          (2.0 * l + 1.0) * _ls_norm_factor * _harmonics.col(harmonics_idx);
      }
      ++harmonics_idx;
    }
  }
}

void
SNScattering::computeJacobian()
{
  prepareMatrixTag(_assembly, _var.number(), _var.number());

  precalculateJacobian();
  for (_qp = 0; _qp < _qrule->n_points(); _qp++)
  {
    initQpJacobian();
    for (_i = 0; _i < _test.size(); _i++)
    {
      _lhs = _tau_sn[_qp][_group] * _ordinates * _array_grad_test[_i][_qp] +
        RealEigenVector::Constant(_count, _test[_i][_qp]);
      for (_j = 0; _j < _phi.size(); _j++)
      {
        _work_vector = _lhs.cwiseProduct(_jac_rhs) * _phi[_j][_qp] * _JxW[_qp] * _coord[_qp];
        _assembly.saveDiagLocalArrayJacobian(
            _local_ke, _i, _test.size(), _j, _phi.size(), _var.number(), _work_vector);
      }
    }
  }

  accumulateTaggedLocalMatrix();

  if (_has_diag_save_in)
  {
    DenseVector<Number> diag = _assembly.getJacobianDiagonal(_local_ke);
    for (const auto & var : _diag_save_in)
    {
      auto * avar = dynamic_cast<ArrayMooseVariable *>(var);
      if (avar)
        avar->addSolution(diag);
      else
        mooseError("Save-in variable for an array kernel must be an array variable");
    }
  }
}

void
SNScattering::initQpJacobian()
{
  _jac_rhs.setZero();
  // If using diffusion flux as initial condition, skip computing Jacobian contributions from
  // 0th scattering moment.
  int start_idx = 0;
  if (_acceleration || (_use_initial_flux && relativeFuzzyEqual(_iteration_postprocessor, 1.0)))
    ++start_idx;
  int harmonics_idx = start_idx;
  for (int l = start_idx; l < _L+1; ++l)
  {
    int scatter_idx = l * _num_groups * _num_groups + _group * _num_groups + _group;
    for (int m = -l; m < l+1; ++m)
    {
      _jac_rhs -= _scatter[_qp][scatter_idx] * (2.0 * l + 1.0) * _ls_norm_factor *
        _harmonics.col(harmonics_idx).cwiseProduct(_harmonics.col(harmonics_idx));
      ++harmonics_idx;
    }
  }
  _jac_rhs = _weights.cwiseProduct(_weights).cwiseProduct(_jac_rhs);
}

RealEigenMatrix
SNScattering::computeQpOffDiagJacobian(const MooseVariableFEBase & jvar)
{
  RealEigenVector lhs = _tau_sn[_qp][_group] * _ordinates * _array_grad_test[_i][_qp] +
    RealEigenVector::Constant(_count, _test[_i][_qp]);

  for (unsigned int g = 0; g < _num_groups; ++g)
  {
    if (jvar.number() == _flux_ids[g])
    {
      RealEigenMatrix jac = RealEigenMatrix::Zero(_count, _count);

      // If using diffusion flux as initial condition, skip computing Jacobian contributions from
      // 0th scattering moment.
      int start_idx = 0;
      int idx = 0;
      if (_acceleration ||
          (_use_initial_flux && relativeFuzzyEqual(_iteration_postprocessor, 1.0)))
      {
        ++start_idx;
        ++idx;
      }
      for (int l = start_idx; l < _L+1; ++l)
      {
        int scatter_idx = l * _num_groups * _num_groups + g * _num_groups + _group;
        if (_scatter[_qp][scatter_idx] == 0.0)
          continue;
        for (int m = -l; m < l+1; ++m)
          for (unsigned int i = 0; i < _count; ++i)
            for (unsigned int j = 0; j < _count; ++j)
              jac(i, j) -= _weights(i) * lhs(i) * _scatter[_qp][scatter_idx] * _weights(j) *
                _harmonics(j, idx) * _phi[_j][_qp] * (2.0 * l + 1.0) * _ls_norm_factor *
                _harmonics(j, idx);
      }
      return jac;
    }
  }
  return ArrayKernel::computeQpOffDiagJacobian(jvar);
}
