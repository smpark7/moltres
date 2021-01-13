#ifndef VELOCITYFUNCTIONADVECTION_H
#define VELOCITYFUNCTIONADVECTION_H

#include "Kernel.h"

// Forward Declaration
class VelocityFunctionAdvection;

template <>
InputParameters validParams<VelocityFunctionAdvection>();

class VelocityFunctionAdvection : public Kernel
{
public:
  VelocityFunctionAdvection(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() override;
  virtual Real computeQpJacobian() override;

  const Function & _vel_x_func;
  const Function & _vel_y_func;
  const Function & _vel_z_func;
};

#endif // VELOCITYFUNCTIONADVECTION_H
