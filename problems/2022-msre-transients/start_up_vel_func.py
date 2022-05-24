import pandas as pd
from scipy.optimize import curve_fit
import numpy as np
import matplotlib.pyplot as plt

vel_data = pd.read_excel('start_up_flow_rate.xlsx', header=None).to_numpy()

t = np.array(vel_data[0][:])
vel = np.array(vel_data[1][:])

t = np.append(t, 10)
vel = np.append(vel, 100)

def func(x, a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r):
    return 100 * (
        a / (1 + np.exp(-b*(x-c))) + d / (1 + np.exp(-e*(x-f))) +
        g / (1 + np.exp(-h*(x-i))) + j / (1 + np.exp(-k*(x-l)))) + \
            m / (1 + np.exp(-n*(x-o))) + p / (1 + np.exp(-q*(x-r)))

def func_2(t):
    return (100 * ( 0.38972176921688984 / (1 + np.exp(- 4.142965323851681 * (t - 2.1997827088970765 ))) + 0.13469852389267972 / (1 + np.exp(- 15.999999999891843 * (t - 1.8229607556265044 ))) + 0.024364484449346135 / (1 + np.exp(- 3.3040453278344293 * (t - 8.261191902527791 ))) + 0.13064766354202037 / (1 + np.exp(- 15.999999999991578 * (t - 1.427099833340688 )))) + 15.999999999999998 / (1 + np.exp(- 3.160549381680319 * (t - 3.0457517232294262 ))) + 15.999999999999998 / (1 + np.exp(- 1.7598149179499631 * (t - 4.365843500245082 ))))

# bounds = 20
# max_error = 1
# for ii in range(10, 30):
#     try:
#         popt, pcov = curve_fit(func, t, vel, bounds=(0, ii))
#     except:
#         continue
#     if max(abs(func(t, *popt) - vel)) < max_error:
#         bounds = ii
#         max_error = max(abs(func(t, *popt) - vel))

t_fine = np.linspace(0, 10, 21)
popt, pcov = curve_fit(func, t, vel, bounds=(0., 16))
fig, ax = plt.subplots()
ax.plot(t, vel, marker='.', linestyle='')
ax.plot(t_fine, func(t_fine, *popt))
ax.plot(t_fine, func_2(t_fine))
ax.set_xlabel('Time [s]')
ax.set_ylabel('Start up velocity [%]')
plt.savefig('start-up.png', dpi=300)

a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r = popt
print('21.45 * 0.01 * (100 * (', a, '/ (1 + exp(-', b, '* (t -', c, '))) +', d,
      '/ (1 + exp(-', e, '* (t -', f, '))) +', g, '/ (1 + exp(-', h, '* (t -',
      i, '))) +', j, '/ (1 + exp(-', k, '* (t -', l, ')))) +', m,
      '/ (1 + exp(-', n, '* (t -', o, '))) +', p, '/ (1 + exp(-', q, '* (t -',
      r, '))))')
