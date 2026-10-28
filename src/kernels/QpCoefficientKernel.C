#include "QpCoefficientKernel.h"

InputParameters
QpCoefficientKernel::validParams()
{
  return KernelValue::validParams();
}

QpCoefficientKernel::QpCoefficientKernel(const InputParameters & parameters)
  : KernelValue(parameters)
{
}

void
QpCoefficientKernel::computeOffDiagJacobian(const unsigned int jvar_num)
{
  if (jvar_num == _var.number())
  {
    computeJacobian();
    return;
  }

  const auto & jvar = getVariable(jvar_num);
  if (jvar.dofIndices().empty())
    return;
  mooseAssert(jvar.count() == 1, "Array variables are not supported");

  _offdiag_coef.resize(_qrule->n_points());
  if (!precomputeOffDiagCoefficients(jvar_num))
    return;

  prepareMatrixTag(_assembly, _var.number(), jvar_num);
  const auto phi_size = jvar.phiSize();
  const unsigned int n_test = _test.size();
  for (_qp = 0; _qp < _qrule->n_points(); _qp++)
  {
    const Real coef = _offdiag_coef[_qp] * _JxW[_qp] * _coord[_qp];
    for (_j = 0; _j < phi_size; _j++)
    {
      const Real value = coef * _phi[_j][_qp];
      for (_i = 0; _i < n_test; _i++)
        _local_ke(_i, _j) += value * _test[_i][_qp];
    }
  }
  accumulateTaggedLocalMatrix();
}
