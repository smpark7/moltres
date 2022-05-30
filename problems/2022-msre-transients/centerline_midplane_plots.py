import numpy as np
import matplotlib.pyplot as plt
import pandas as pd

# %% Flux plots

centerline_flux = {'group1': np.array(
    pd.read_csv(
        'msre-static-pp_csv_group1_centerline_0023.csv')['group1_source']),
           'group2': np.array(
    pd.read_csv(
        'msre-static-pp_csv_group2_centerline_0023.csv')['group2_source'])
        }

y = np.linspace(0, 170, len(centerline_flux['group1']))
fig, ax = plt.subplots()
ax.plot(y, centerline_flux['group1'],
        label='Group 1 flux', marker='.', linestyle='')
ax.plot(y, centerline_flux['group2'],
        label='Group 2 flux', marker='.', linestyle='')
ax.legend()
ax.set_xlabel('Height [cm]')
ax.set_ylabel(r'Neutron flux [# cm$^{-2}$ s$^{-1}$]')

midplane_flux = {'group1': np.array(
    pd.read_csv(
        'msre-static-pp_csv_group1_midplane_0023.csv')['group1_source']),
           'group2': np.array(
    pd.read_csv(
        'msre-static-pp_csv_group2_midplane_0023.csv')['group2_source'])
        }

x = np.linspace(0, 69.375, len(midplane_flux['group1']))
fig, ax = plt.subplots()
ax.plot(x, midplane_flux['group1'], label='Group 1 flux')
ax.plot(x, midplane_flux['group2'], label='Group 2 flux')
ax.legend()
ax.set_xlabel('Radius [cm]')
ax.set_ylabel(r'Neutron flux [# cm$^{-2}$ s$^{-1}$]')

pd.DataFrame(centerline_flux, index=y).to_csv('centerline_flux.csv',
                                              index_label='height')
pd.DataFrame(midplane_flux, index=x).to_csv('midplane_flux.csv',
                                            index_label='radius')

# %% Precursor plots

centerline_pre = {
    'pre1': np.array(
        pd.read_csv(
            'msre-static-pp_csv_pre1_centerline_0023.csv')['pre1']),
    'pre2': np.array(
        pd.read_csv(
            'msre-static-pp_csv_pre2_centerline_0023.csv')['pre2']),
    'pre3': np.array(
        pd.read_csv(
            'msre-static-pp_csv_pre3_centerline_0023.csv')['pre3']),
    'pre4': np.array(
        pd.read_csv(
            'msre-static-pp_csv_pre4_centerline_0023.csv')['pre4']),
    'pre5': np.array(
        pd.read_csv(
            'msre-static-pp_csv_pre5_centerline_0023.csv')['pre5']),
    'pre6': np.array(
        pd.read_csv(
            'msre-static-pp_csv_pre6_centerline_0023.csv')['pre6']),
    }

midplane_pre = {
    'pre1': np.array(
        pd.read_csv(
            'msre-static-pp_csv_pre1_midplane_0023.csv')['pre1']),
    'pre2': np.array(
        pd.read_csv(
            'msre-static-pp_csv_pre2_midplane_0023.csv')['pre2']),
    'pre3': np.array(
        pd.read_csv(
            'msre-static-pp_csv_pre3_midplane_0023.csv')['pre3']),
    'pre4': np.array(
        pd.read_csv(
            'msre-static-pp_csv_pre4_midplane_0023.csv')['pre4']),
    'pre5': np.array(
        pd.read_csv(
            'msre-static-pp_csv_pre5_midplane_0023.csv')['pre5']),
    'pre6': np.array(
        pd.read_csv(
            'msre-static-pp_csv_pre6_midplane_0023.csv')['pre6']),
    }

y = np.linspace(1.25, 168.75, len(centerline_pre['pre1']))
fig, ax = plt.subplots()
ax.plot(y, centerline_pre['pre1'],
        label='Precursor group 1', marker='.', linestyle='')
ax.legend()
ax.set_xlabel('Height [cm]')
ax.set_ylabel(r'Precursor distribution [# cm$^3$]')

for j in range(1, 7):
    x = np.linspace(0.0390625, 69.3359375, len(midplane_pre['pre'+str(j)]))
    counter = 0
    for i in range(len(midplane_pre['pre'+str(j)])):
        if counter < 48:
            midplane_pre['pre'+str(j)][-i-1] = 0
            counter += 1
        elif counter < 63:
            counter += 1
        else:
            counter = 0
    fig, ax = plt.subplots()
    ax.plot(x, midplane_pre['pre'+str(j)],
            label='Precursor group '+str(j), marker=',', linestyle='')
    ax.legend()
    ax.set_xlabel('Radius [cm]')
    ax.set_ylabel(r'Precursor distribution [# cm$^3$]')

pd.DataFrame(centerline_pre, index=y).to_csv('centerline_pre.csv',
                                             index_label='height')
pd.DataFrame(midplane_pre, index=x).to_csv('midplane_pre.csv',
                                           index_label='radius')
