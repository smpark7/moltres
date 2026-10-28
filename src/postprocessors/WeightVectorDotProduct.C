#include "WeightVectorDotProduct.h"
#include "FissionWeightVector.h"
#include "SystemBase.h"

registerMooseObject("MoltresApp", WeightVectorDotProduct);

InputParameters
WeightVectorDotProduct::validParams()
{
  InputParameters params = GeneralPostprocessor::validParams();
  params.addClassDescription("Dot product of a FissionWeightVector's weights with the current "
                             "solution, i.e. its fission integral.");
  params.addRequiredParam<UserObjectName>("weight_vector", "The FissionWeightVector user object.");
  return params;
}

WeightVectorDotProduct::WeightVectorDotProduct(const InputParameters & parameters)
  : GeneralPostprocessor(parameters),
    _weight_vector(getUserObject<FissionWeightVector>("weight_vector")),
    _value(0)
{
}

void
WeightVectorDotProduct::execute()
{
  // EigenProblem evaluates bx_norm to scale the initial eigenvector before the initial user objects
  // run, so build W here if it is not built yet
  if (!_weight_vector.built())
  {
    _fe_problem.computeUserObjectByName(EXEC_INITIAL, Moose::POST_AUX, _weight_vector.name());
    if (!_weight_vector.built())
      mooseError("Could not build the weights of '", _weight_vector.name(), "'.");
  }
  // currentSolution() is the iterate being evaluated, including during residual evaluations
  _value = _weight_vector.weights().dot(*_weight_vector.system().currentSolution());
}
