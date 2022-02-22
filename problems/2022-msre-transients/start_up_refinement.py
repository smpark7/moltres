import numpy as np
import matplotlib.pyplot as plt
import pandas as pd

# %% Reactivity function
def get_rho(initial, moltres_data):
    # Moltres data
    initial = np.array(pd.read_csv(initial)['bnorm'])[-1]
    initial_rho = (initial - 1) / initial

    k_eff = np.array(pd.read_csv(moltres_data)['bnorm'])
    t = np.array(pd.read_csv(moltres_data)['time'])
    num_steps = len(k_eff)
    rho = np.zeros(num_steps)
    k_eff[0] = initial

    for i in range(num_steps):
        rho[i] = (initial_rho - (k_eff[i] - 1) / k_eff[i]) * 1e5

    return t, rho

# %% Start-up Moltres data

t1, rho1 = get_rho('msre-static_csv.csv',
                  'msre-startup-flow_csv (2).csv')
t2, rho2 = get_rho('msre-static_csv.csv',
                   'msre-startup-flow_csv (5).csv')
t3, rho3 = get_rho('msre-static_csv.csv',
                   'msre-startup-time-refine-flow_csv.csv')
t4, rho4 = get_rho('msre-static_csv.csv',
                   'msre-startup-prec-refine-flow_csv.csv')
t5, rho5 = get_rho('msre-static_csv.csv',
                   'msre-startup-prec-refine-flow_csv (2).csv')

fig, ax = plt.subplots()
ax.plot(t1, rho1, label='dt=1s, dz=5cm')
ax.plot(t2, rho2, label='dt=0.5s, dz=5cm')
ax.plot(t3, rho3, label='dt=0.25s, dz=5cm')
ax.plot(t4, rho4, linestyle='--', label='dt=0.5s, dz=2.5cm')
ax.plot(t5, rho5, linestyle='--', label='dt=0.25s, dz=2.5cm')
ax.set_ylabel('Reactivity of rod withdrawal [pcm]')
ax.set_xlabel('Time [s]')
ax.legend()
ax.set_xlim(0, 100)
ax.set_ylim(0, 400)
plt.grid()
plt.savefig('start-up-refinement.png', dpi=300)
