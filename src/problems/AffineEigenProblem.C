#include "AffineEigenProblem.h"

#include "NonlinearEigenSystem.h"
#include "SlepcSupport.h"

#include "libmesh/petsc_matrix.h"
#include "libmesh/petsc_vector.h"

registerMooseObject("MoltresApp", AffineEigenProblem);

namespace
{
AffineEigenProblem &
affineProblem(void * ctx)
{
  return *static_cast<AffineEigenProblem *>(static_cast<EigenProblem *>(ctx));
}

PetscErrorCode
affineFormFunctionA(SNES snes, Vec x, Vec r, void * ctx)
{
  PetscFunctionBegin;
  PetscCall(Moose::SlepcSupport::mooseSlepcEigenFormFunctionA(snes, x, r, ctx));
  auto & problem = affineProblem(ctx);
  if (problem.matrixResidualActive())
    PetscCall(problem.addAffineTerm(x, r));
  PetscFunctionReturn(PETSC_SUCCESS);
}

PetscErrorCode
affineFormFunctionAB(SNES snes, Vec x, Vec Ax, Vec Bx, void * ctx)
{
  PetscFunctionBegin;
  PetscCall(Moose::SlepcSupport::mooseSlepcEigenFormFunctionAB(snes, x, Ax, Bx, ctx));
  auto & problem = affineProblem(ctx);
  if (problem.matrixResidualActive())
    PetscCall(problem.addAffineTerm(x, Ax));
  PetscFunctionReturn(PETSC_SUCCESS);
}
}

InputParameters
AffineEigenProblem::validParams()
{
  InputParameters params = EigenProblem::validParams();
  params.addClassDescription(
      "EigenProblem that retains solution-independent residual terms (e.g. an AuxVariable "
      "delayed neutron source) under PJFNKMO with constant_matrices = true.");
  return params;
}

AffineEigenProblem::AffineEigenProblem(const InputParameters & parameters)
  : EigenProblem(parameters)
{
}

AffineEigenProblem::~AffineEigenProblem()
{
  if (_affine)
    (void)VecDestroy(&_affine);
}

bool
AffineEigenProblem::matrixResidualActive()
{
  const auto & nl = getCurrentNonlinearEigenSystem();
  return solverParams(nl.number())._eigen_matrix_vector_mult &&
         (onLinearSolver() || constantMatrices());
}

void
AffineEigenProblem::initPetscOutputAndSomeSolverSettings()
{
  EigenProblem::initPetscOutputAndSomeSolverSettings();

  // A new EPSSolve follows: aux fields may have changed since the last one
  _affine_valid = false;

  auto & nl = getCurrentNonlinearEigenSystem();
  if (nl.sys().get_dof_map().n_constrained_dofs())
    mooseError("AffineEigenProblem does not support constrained dofs");
  if (!nl.sys().has_matrix_A())
    return;

  // SLEPc reads these from A in EPSSetUp, which runs inside the upcoming EPSSolve
  Mat A = static_cast<libMesh::PetscMatrix<Number> &>(nl.sys().get_matrix_A()).mat();
  LibmeshPetscCall(PetscObjectComposeFunction((PetscObject)A, "formFunction", affineFormFunctionA));
  LibmeshPetscCall(
      PetscObjectComposeFunction((PetscObject)A, "formFunctionAB", affineFormFunctionAB));
}

PetscErrorCode
AffineEigenProblem::addAffineTerm(Vec x, Vec Ax)
{
  PetscFunctionBegin;
  if (!_affine_valid)
  {
    // Ax currently holds MatMult(A, x); a full residual at the same x gives A x + c
    auto & nl = getCurrentNonlinearEigenSystem();
    auto & sys = nl.sys();

    if (!_affine)
      PetscCall(VecDuplicate(Ax, &_affine));

    PetscVector<Number> X_global(x, sys.comm());
    PetscVector<Number> & X_sys = *cast_ptr<PetscVector<Number> *>(sys.solution.get());
    X_global.swap(X_sys);
    sys.update();
    X_global.swap(X_sys);

    PetscVector<Number> R(_affine, sys.comm());
    R.zero();
    computeResidualTag(*sys.current_local_solution, R, nl.nonEigenVectorTag());
    R.close();

    // c = F_A(x) - A x
    PetscCall(VecAXPY(_affine, -1.0, Ax));
    _affine_valid = true;
  }
  PetscCall(VecAXPY(Ax, 1.0, _affine));
  PetscFunctionReturn(PETSC_SUCCESS);
}
