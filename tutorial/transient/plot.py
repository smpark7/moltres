import numpy as np
import pandas as pd
import matplotlib.pyplot as plt

data = pd.read_csv('transient_csv.csv')

fig, ax = plt.subplots(dpi=300)
ax.plot(data['time'][1:], data['powernorm'][1:]/1e6)
ax.set_xscale('log')
ax.legend(['Total power output'])
ax.set_xlabel('Time [s]')
ax.set_ylabel('Power [MW]')

fig, ax = plt.subplots(dpi=300)
ax.plot(data['time'][1:], data['inlet_temp'][1:])
ax.plot(data['time'][1:], data['outlet_temp'][1:])
ax.plot(data['time'][1:], data['average_temp'][1:])
ax.set_xscale('log')
ax.legend(['Inlet temperature', 'Outlet temperature', 'Average Temperature'])
ax.set_xlabel('Time [s]')
ax.set_ylabel('Temperature [K]')
