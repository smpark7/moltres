#include "NtTimeDerivative.h"
#include "Assembly.h"

// libmesh includes
#include "libmesh/quadrature.h"

registerMooseObject("MoltresApp", NtTimeDerivative);

InputParameters
NtTimeDerivative::validParams()
{
  InputParameters params = ScalarTransportTimeDerivative::validParams();
  params.addRequiredParam<unsigned int>("group_number",
                                        "The group for which this kernel controls diffusion");
  params.addCoupledVar("temperature",
                       "The temperature used to interpolate the diffusion coefficient");
  return params;
}

NtTimeDerivative::NtTimeDerivative(const InputParameters & parameters)
  : ScalarTransportTimeDerivative(parameters),
    _recipvel(getMaterialProperty<std::vector<Real>>("recipvel")),
    _d_recipvel_d_temp(getMaterialProperty<std::vector<Real>>("d_recipvel_d_temp")),
    _group(getParam<unsigned int>("group_number") - 1),
    _temp_id(coupled("temperature"))
{
}

void
NtTimeDerivative::computeResidual()
{
  prepareVectorTag(_assembly, _var.number());

  const unsigned int n_test = _test.size();
  for (_qp = 0; _qp < _qrule->n_points(); _qp++)
  {
    const Real value = computeConcentrationDot(_u, _u_dot, _qp) * _conc_scaling *
                       _recipvel[_qp][_group] * _JxW[_qp] * _coord[_qp];
    for (_i = 0; _i < n_test; _i++)
      _local_re(_i) += value * _test[_i][_qp];
  }

  computeResidualAdditional();

  accumulateTaggedLocalResidual();

  if (_has_save_in)
    for (const auto & var : _save_in)
      var->sys().solution().add_vector(_local_re, var->dofIndices());
}

void
NtTimeDerivative::computeJacobian()
{
  prepareMatrixTag(_assembly, _var.number(), _var.number());

  const unsigned int n_test = _test.size();
  for (_qp = 0; _qp < _qrule->n_points(); _qp++)
  {
    const Real coef = _conc_scaling * _recipvel[_qp][_group] * _JxW[_qp] * _coord[_qp];
    for (_j = 0; _j < _phi.size(); _j++)
    {
      const Real value =
          computeConcentrationDotDerivative(_u, _u_dot, _du_dot_du, _phi, _j, _qp) * coef;
      for (_i = 0; _i < n_test; _i++)
        _local_ke(_i, _j) += value * _test[_i][_qp];
    }
  }

  accumulateTaggedLocalMatrix();

  if (_has_diag_save_in && !_sys.computingScalingJacobian())
  {
    DenseVector<Number> diag = _assembly.getJacobianDiagonal(_local_ke);
    for (const auto & var : _diag_save_in)
      var->sys().solution().add_vector(diag, var->dofIndices());
  }
}

void
NtTimeDerivative::computeOffDiagJacobian(const unsigned int jvar)
{
  if (jvar == _var.number())
    computeJacobian();
  else if (jvar == _temp_id)
    ScalarTransportTimeDerivative::computeOffDiagJacobian(jvar);
}

Real
NtTimeDerivative::computeQpResidual()
{
  return ScalarTransportTimeDerivative::computeQpResidual() * _recipvel[_qp][_group];
}

Real
NtTimeDerivative::computeQpJacobian()
{
  return ScalarTransportTimeDerivative::computeQpJacobian() * _recipvel[_qp][_group];
}

Real
NtTimeDerivative::computeQpOffDiagJacobian(unsigned int jvar)
{
  if (jvar == _temp_id)
    return ScalarTransportTimeDerivative::computeQpResidual() * _d_recipvel_d_temp[_qp][_group] *
           _phi[_j][_qp];

  else
    return 0;
}
