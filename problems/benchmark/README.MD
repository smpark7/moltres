## Multiphysics Numerical Benchmark Case 1.1

Ref: Results from a multi-physics numerical benchmark for code dedicated to
molten salt fast reactors

This directory holds cross section data and input files to run case 1.1 in the
aforementioned paper. The results should show that the Discontinuous Galerkin
method provides a more accurate result than the Continuous Galerkin method,
with the Polimi model as reference.

The CG method for resolving precursor concentration was implemented because
the DG method resulted in inaccurate boundary values compared to the Polimi
model. However, the CG method provided worse results overall because the
artificial diffusion kernel (ScalarAdvectionArtDiff) is overly diffusive.

The CG method should perform better with a more advanced stabilization
method, such as SUPG or FIC. Refer to Stabilization techniques for finite
element analysis by Onate & Manzan, and the COMSOL manual for more information
and derivation.

### How to run

1. Compile Moltres with this current branch that allows for continuous,
first-order precursor concentration variables.
2. Run ```mpirun -np 2 ../../moltres-opt -i vel-field-stabilized.i``` to get
the velocity field in a lid-driven cavity.
3. Run ```mpirun -np 2 ../../moltres-opt -i circ-fuel.i``` to get the
neutronics results in the cavity under the velocity field from step (2) with
the discontinuous Galerkin method for precursors.
4. Run ```mpirun -np 2 ../../moltres-opt -i circ-fuel-first.i``` to get the
neutronics results in the cavity under the velocity field from step (2) with
the continuous Galerkin method for precursors.
5. Run ```python3 circ-fuel.py``` to get the horizontal and vertical midline
delayed neutron source from both set of results, compared against the
reference results from Polimi.

Run the input files on a 200 by 200 mesh if you have sufficient computational
resources for more accurate results (CG still performs worse).
