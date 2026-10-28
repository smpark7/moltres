#pragma once

#include "KernelValue.h"

/**
 * KernelValue whose off-diagonal Jacobian is also of the form
 *
 *   JxW[_qp] * coef[_qp] * _phi[_j][_qp] * _test[_i][_qp]
 *
 * Derived classes fill the per-qp coefficients once per element and coupled variable instead of
 * recomputing them for every test and shape function; blocks that are identically zero are
 * skipped.
 */
class QpCoefficientKernel : public KernelValue
{
public:
  static InputParameters validParams();

  QpCoefficientKernel(const InputParameters & parameters);

  virtual void computeOffDiagJacobian(unsigned int jvar) override;

protected:
  /**
   * Fills _offdiag_coef (sized to the number of quadrature points) for coupled variable \p jvar
   * @return false if the block is identically zero, in which case _offdiag_coef is not used
   */
  virtual bool precomputeOffDiagCoefficients(unsigned int jvar) = 0;

  /// Per-qp off-diagonal Jacobian coefficients for the current coupled variable
  std::vector<Real> _offdiag_coef;
};
