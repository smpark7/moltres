#ifndef FISSIONEHEATSOURCEAUX_H
#define FISSIONEHEATSOURCEAUX_H

#include "AuxKernel.h"

//Forward Declarations
class FissionHeatSourceAux;

template<>
InputParameters validParams<FissionHeatSourceAux>();

class FissionHeatSourceAux : public AuxKernel
{
public:
  FissionHeatSourceAux(const InputParameters & parameters);

protected:
  virtual Real computeValue();

  const MaterialProperty<std::vector<Real> > & _fissxs;
  const MaterialProperty<std::vector<Real> > & _d_fissxs_d_temp;
  unsigned int _num_groups;
  const PostprocessorValue & _tot_fissions;
  Real _power;
  std::vector<const VariableValue *> _group_fluxes;
  std::vector<unsigned int> _flux_ids;
};

#endif //FISSIONEHEATSOURCE_H
