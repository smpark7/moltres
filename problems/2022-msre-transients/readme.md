# MSRE Coast-down & Start-up Transients

This directory contains the input files for the MSRE coast-down and start-up
transients in collaboration with Dr Aaron Reynolds.

## Input Files

### Mesh Generation

`mesh.i` generates the Exodus mesh file for the MSRE simulations. Run with
the following command:

`$MOLTRES/moltres-opt -i mesh.i --mesh-only mesh.e`

### Group Constant Generation

`xsdata.py` produces the group constant data in a JSON file format compatible
with Moltres. Run with `python3`.

### Standalone Neutronics Solve for Mesh Refinement

- `msre-nts.i`: Fast neutronics criticality solve of the static system. Used
for mesh refinement study. Run with the following command:

`mpirun -n 4 $MOLTRES/moltres-opt -i msre-nts.i`

### MSRE Transients

All MSRE transients are run on segregated solves coupled via Picard
iterations. The primary input files (to be run with the Moltres executable)
simulate time-dependent flow velocity and precursor drift. The primary input
files automatically couple to the `msre-transient-nts.i` file which performs a
criticality solve at every time-step (and Picard iteration). They also
automatically couple to the`loop` files which simulate precursor drift through
the recirculation region.

#### Primary Input Files

- `msre-steady-state-flow.i`: Primary input file for simulating the MSRE under
steady-flow (v = 21.45cm/s) conditions. To be used as initial conditions
for the coast-down transient.
- `msre-static.i`: Primary input file for simulating the MSRE under static
(v = 0cm/s) conditions. To be used as initial conditions for the start-up
transient. Does not couple to a `loop` file because precursors are stationary.
- `msre-coastdown-flow.i`: Primary input file for simulating the coast-down
transient.
- `msre-startup-flow.i`: Primary input file for simulating the start-up
transient.o

Run with the following command:
`mpirun -n 4 $MOLTRES/moltres-opt -i input-file.i`

#### Secondary Input Files

- `msre-transient-nts.i`: Neutronics criticality solver to calculate k at every
time-step during the transients. Required by all primary input files.
- `msre-steady-state-loop.i`: Simulates precursor drift in the recirculation
region under steady-flow (v = 21.45cm/s) conditions.
- `msre-coastdown-loop.i`: Simulates precursor drift in the recirculation
region during the coast-down transient.
- `msre-startup-loop.i`: Simulates precursor drift in the recirculation region
during the start-up transient.

#### Output Files

The CSV output files report the k value for every time-step as `bnorm`. The
Exodus output files show the neutron fluxes & precursor concentrations at every
time-step for the primary (*_exodus.e), nts (*_out_ntsApp0_exodus.e), and loop
(*_out_loopApp0_exodus.e) simulations.
