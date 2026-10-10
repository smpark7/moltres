#pragma once

#include "EigenProblem.h"

/**
 * EigenProblem that keeps residual contributions which are constant in the solution
 * (e.g. a delayed neutron source coupled as an AuxVariable) when solving with
 * solve_type = PJFNKMO and constant_matrices = true.
 *
 * In that mode MOOSE evaluates A(x) as MatMult(A, x), which drops any affine part of
 * the non-eigen residual. With the aux fields frozen during an eigen solve,
 * F_A(x) = A x + c holds exactly, so c is recovered once per solve from one full
 * residual evaluation and added back after every MatMult.
 */
class AffineEigenProblem : public EigenProblem
{
public:
  static InputParameters validParams();

  AffineEigenProblem(const InputParameters & parameters);
  ~AffineEigenProblem();

  /// Called by SlepcEigenSolverConfiguration after MOOSE attaches its SLEPc callbacks
  /// and before EPSSolve; re-attaches the residual callbacks with affine wrappers
  virtual void initPetscOutputAndSomeSolverSettings() override;

  /// Whether MOOSE evaluated A x with MatMult in the current callback
  bool matrixResidualActive();

  /// Add c to Ax, computing c from a full residual at x first if needed
  PetscErrorCode addAffineTerm(Vec x, Vec Ax);

protected:
  /// Affine part of the non-eigen residual; recomputed on every eigen solve
  Vec _affine = nullptr;
  bool _affine_valid = false;
};
