import numpy as np
import matplotlib.pyplot as plt
import pandas as pd
import scipy.interpolate as spi

#%% Read CSV data for DG and CG delayed neutron precursors

aa = pd.read_csv('circ-fuel_out_aa_0002.csv')
bb = pd.read_csv('circ-fuel_out_bb_0002.csv')

aa_first = pd.read_csv('circ-fuel-first_out_aa_0002.csv')
bb_first = pd.read_csv('circ-fuel-first_out_bb_0002.csv')

#%% Convert concentration to delayed neutron source

# Decay constant, lambda
lam = [0.0124667, 0.0282917, 0.0425244, 0.133042,
       0.292467, 0.666488, 1.63478, 3.5546]

# x from 0cm to 200cm at 25cm intervals
x = np.arange(0, 201, 25)

aa_pre = np.zeros(len(x))
bb_pre = np.zeros(len(x))

# Multiply concentrations with the corresponding lambda and the power
# normalization factor (DG)
for i in range(len(x)):
    for j in range(8):
        aa_pre[i] += aa['pre' + str(j+1)][x[i]] * lam[j] * 1e6 * 7.53085e17
        bb_pre[i] += bb['pre' + str(j+1)][x[i]] * lam[j] * 1e6 * 7.53085e17

aa_first_pre = np.zeros(len(x))
bb_first_pre = np.zeros(len(x))

# Multiply concentrations with the corresponding lambda and the power
# normalization factor (CG)
for i in range(len(x)):
    for j in range(8):
        aa_first_pre[i] += aa_first['pre' + str(j+1)][x[i]] * lam[j] * 1e6 * 7.53085e17
        bb_first_pre[i] += bb_first['pre' + str(j+1)][x[i]] * lam[j] * 1e6 * 7.53085e17

# Reference data from Polimi model
polimi_aa = [1.321e16, 1.450e17, 2.219e17, 2.414e17, 2.266e17,
             1.920e17, 1.459e17, 9.188e16, 1.292e16]
polimi_bb = [1.297e16, 1.186e17, 1.881e17, 2.194e17, 2.266e17,
             2.260e17, 2.177e17, 1.756e17, 2.805e16]

#%% Plot data
x = [0, .25, .5, .75, 1, 1.25, 1.5, 1.75, 2]
fig, ax = plt.subplots()
ax.plot(x, aa_pre, label='Moltres (Discontinuous Galerkin')
ax.plot(x, aa_first_pre, label='Moltres (Continuous Galerkin)')
ax.plot(x, polimi_aa, label='Polimi')
ax.legend()
ax.set_xlabel(r'$x$ [m]')
ax.set_ylabel(r'Delayed neutron source [m$^{-3}\cdot$s$^{-1}$]')
plt.savefig('dnp-aa.png', dpi=400)

fig, ax = plt.subplots()
ax.plot(x, bb_pre, label='Moltres (Discontinuous Galerkin)')
ax.plot(x, bb_first_pre, label='Moltres (Continuous Galerkin)')
ax.plot(x, polimi_bb, label='Polimi')
ax.legend()
ax.set_xlabel(r'$y$ [m]')
ax.set_ylabel(r'Delayed neutron source [m$^{-3}\cdot$s$^{-1}$]')
plt.savefig('dnp-bb.png', dpi=400)
