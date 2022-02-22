import numpy as np
import matplotlib.pyplot as plt
import pandas as pd

# %% Reactivity function
def get_rho(static, initial, moltres_data, msre_data, qm_data):
    # Moltres data
    static = np.array(pd.read_csv(static)['bnorm'])[-1]
    static_rho = (static - 1) / static

    k_eff = np.array(pd.read_csv(moltres_data)['bnorm'])
    t = np.array(pd.read_csv(moltres_data)['time'])
    num_steps = len(k_eff)
    rho = np.zeros(num_steps)
    k_eff[0] = np.array(pd.read_csv(initial)['bnorm'])[-1]

    for i in range(num_steps):
        rho[i] = (static_rho - (k_eff[i] - 1) / k_eff[i]) * 1e5

    # MSRE data
    df = pd.read_csv(msre_data, header=None)

    # QuasiMolto data
    qm = np.array(pd.read_csv(qm_data, header=None))

    return t, rho, df, qm

# %% Coast-down Moltres data
t, rho, df, qm = get_rho('msre-static_csv.csv',
                     'msre-steady-state-flow_csv (2).csv',
                     'msre-coastdown-flow_csv (2).csv',
                     'ref_coast_down_reactivity.csv',
                     'qm_coast_down_reactivity.csv')

np.savetxt('moltres_coast_down_reactivity.csv', (t, rho), delimiter=',')

fig, ax = plt.subplots()
ax.plot(qm[0], qm[1], marker='.', linestyle='', label='QuasiMolto')
ax.plot(t, rho, marker='.', linestyle='', label='Moltres')
ax.plot(df[0], df[1], marker='.', linestyle='', label='ORNL MSRE Data')
ax.set_ylabel('Reactivity of rod withdrawal [pcm]')
ax.set_xlabel('Time [s]')
ax.legend()
ax.set_xlim(0, 70)
ax.set_ylim(0, 400)
plt.grid()
plt.savefig('coast-down-reactivity.png', dpi=300)

fig, ax = plt.subplots()
ax.plot(qm[0], qm[1]/max(qm[1]), marker='.', linestyle='', label='QuasiMolto')
ax.plot(t, rho / max(rho), marker='.', linestyle='', label='Moltres')
ax.plot(df[0], df[1] / max(df[1]), marker='.', linestyle='',
        label='ORNL MSRE Data')
ax.set_ylabel('Fraction of peak reactivity')
ax.set_xlabel('Time [s]')
ax.legend()
ax.set_xlim(0, 70)
ax.set_ylim(0,)
plt.grid()
plt.savefig('coast-down-fraction.png', dpi=300)

# %% Start-up Moltres data
t, rho, df, qm = get_rho('msre-static_csv.csv',
                     'msre-static_csv.csv',
                     'msre-startup-prec-refine-flow_csv (2).csv',
                     'ref_start_up_reactivity.csv',
                     'qm_start_up_reactivity.csv')

np.savetxt('moltres_start_up_reactivity.csv', (t, rho), delimiter=',')

fig, ax = plt.subplots()
ax.plot(qm.T[0], qm.T[1], marker='.', linestyle='', label='QuasiMolto')
ax.plot(t[::4], rho[::4], marker='.', linestyle='', label='Moltres')
ax.plot(df[0], df[1], marker='.', linestyle='', label='ORNL MSRE Data')
ax.set_ylabel('Reactivity of rod withdrawal [pcm]')
ax.set_xlabel('Time [s]')
ax.legend()
ax.set_xlim(0, 150)
ax.set_ylim(-100, 500)
plt.grid()
plt.savefig('start-up-reactivity.png', dpi=300)

fig, ax = plt.subplots()
ax.plot(qm.T[0], qm.T[1]/max(qm.T[1]), marker='.', linestyle='', label='QuasiMolto')
ax.plot(t, rho / max(rho), marker='.', linestyle='', label='Moltres')
ax.plot(df[0], df[1] / max(df[1]), marker='.', linestyle='',
        label='ORNL MSRE Data')
ax.set_ylabel('Fraction of peak reactivity')
ax.set_xlabel('Time [s]')
ax.legend()
ax.set_xlim(0, 150)
ax.set_ylim(0,)
plt.grid()
plt.savefig('start-up-fraction.png', dpi=300)
