import pandas as pd
from scipy.optimize import curve_fit
import numpy as np
import matplotlib.pyplot as plt

vel_data = pd.read_excel('coast_down_flow_rate.xlsx', header=None).to_numpy()

t = np.array(vel_data[0][:])
vel = np.array(vel_data[1][:])

t = np.append(t, 21)
vel = np.append(vel, 0)

# def func(x, a, b, c, d, e, f, g, h, i, j, k):
#     return a * (x ** 3) * np.exp(- b * x) + c * (x ** 2) * np.exp(- d * x) + \
#         e * x * np.exp(- f * x) + g * np.exp(- h * x) + i * np.exp(- j * x) - k
def func(x, a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r):
    return 100 * (
        a / (1 + np.exp(b*(x-c))) + d / (1 + np.exp(e*(x-f))) +
        g / (1 + np.exp(h*(x-i))) + j / (1 + np.exp(k*(x-l)))) + \
            m / (1 + np.exp(n*(x-o))) - p / (1 + np.exp(-q*(x-r)))

# bounds = 20
# max_error = 1
# for ii in range(20, 50):
#     try:
#         popt, pcov = curve_fit(func, t, vel, bounds=(0, ii))
#     except:
#         continue
#     if max(abs(func(t, *popt) - vel)) < max_error:
#         bounds = ii
#         max_error = max(abs(func(t, *popt) - vel))

t_fine = np.linspace(0, 21, 22)
popt, pcov = curve_fit(func, t, vel, bounds=(0, 25))
fig, ax = plt.subplots()
ax.plot(t, vel, marker='.', linestyle='')
ax.plot(t_fine, func(t_fine, *popt), marker='.', linestyle='')
ax.set_xlabel('Time [s]')
ax.set_ylabel('Coast down velocity [%]')
plt.savefig('coast-down.png', dpi=300)

a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r = popt
print('21.45 * 0.01 * (100 * (', a, '/ (1 + exp(', b, '* (t -', c, '))) +', d,
      '/ (1 + exp(', e, '* (t -', f, '))) +', g, '/ (1 + exp(', h, '* (t -', i,
      '))) +', j, '/ (1 + exp(', k, '* (t -', l, ')))) +', m, '/ (1 + exp(', n,
      '* (t -', o, '))) -', p, '/ (1 + exp(-', q, '* (t -', r, '))))')
# print('21.45 * 0.01 * (', a, '* t^3 * exp(-', b, '* t) +', c, '* t^2 * exp(-',
#       d, '* t)+', e, '* t * exp(-', f, '* t)+', g, '* exp(-', h, '* t) +',
#       i, '* exp(-', j, '* t) -', k, ')')
