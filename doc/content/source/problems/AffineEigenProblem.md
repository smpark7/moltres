# AffineEigenProblem

!syntax description /Problem/AffineEigenProblem

## Overview

With `solve_type = PJFNKMO` and `constant_matrices = true`, the `Eigenvalue` executioner
evaluates the non-eigen residual as $A x$ from a matrix assembled once per solve. Residual
terms that do not depend on the solution, such as a delayed neutron source supplied as an
AuxVariable by a thermal-hydraulics MultiApp through `delayed_neutron_source`, have no
matrix entries and are silently dropped.

`AffineEigenProblem` restores them. Because the auxiliary fields are frozen during an eigen
solve, the non-eigen residual is affine in the solution, $F_A(x) = A x + c$. On the first
matrix-based residual evaluation of each solve, the constant vector $c$ is recovered from one
full residual evaluation, $c = F_A(x) - A x$, and it is added after every subsequent
matrix-vector product. $c$ is recomputed for every eigen solve, so updated auxiliary fields
are picked up on each fixed-point (Picard) iteration.

The problem behaves exactly like `EigenProblem` for all other solve types. Requirements:

- the non-eigen residual must be linear in the solution apart from the constant term
  (e.g. no solid temperature solved in the same system as the neutron fluxes), which is the
  same requirement `constant_matrices` already imposes;
- the eigen (B) residual must not contain solution-independent terms;
- the system must not have constrained degrees of freedom.

## Example Input File Syntax

!listing tests/problems/affine_eigen.i block=Problem Executioner

!syntax parameters /Problem/AffineEigenProblem

!syntax inputs /Problem/AffineEigenProblem

!syntax children /Problem/AffineEigenProblem
