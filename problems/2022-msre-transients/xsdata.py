import json
import numpy as np

# Input data

groups = 2
temps = ['922',]
mats = {
    'fuel': {
        '922': {
            'GTRANSFXS': np.array([.266445, .00202371, .00159430, .249011]),
            'FISSXS': np.array([.00130417, .0217635]),
            'NSF': np.array([.00318268, .0530312]),
            'CHI_T': np.array([1, 0]),
            'CHI_P': np.array([1, 0]),
            'CHI_D': np.array([1, 0]),
            'RECIPVEL': np.array([9.78949e-8, 2.05657e-6]),
            'BETA_EFF': np.array([.000218239, .00142124, .00127070,
                                   .00258174, .000760071, .000262749]),
            'DECAY_CONSTANT': np.array([.0124378, .0306300, .111474,
                                .302248, 1.17229, 3.07474]),
            'sigma_tr': np.array([.233433, .264024]),
            'sigma_a': np.array([.00379970, .0270469]),
            'kappa_fission': np.array([4.05462e-14, 6.76535e-13])
        },
        'temp': [922]
    },
    'mod': {
        '922': {
            'GTRANSFXS': np.array([.393826, .00424578, .00296472, .455480]),
            'FISSXS': np.array([0, 0]),
            'NSF': np.array([0, 0]),
            'CHI_T': np.array([0, 0]),
            'CHI_P': np.array([0, 0]),
            'CHI_D': np.array([0, 0]),
            'RECIPVEL': np.array([9.98046e-8, 2.07824e-6]),
            'BETA_EFF': np.array([0, 0, 0, 0, 0, 0]),
            'DECAY_CONSTANT': np.array([0, 0, 0, 0, 0, 0]),
            'sigma_tr': np.array([.338400, .433088]),
            'sigma_a': np.array([1.41127e-5, 1.44339e-4]),
            'kappa_fission': np.array([0, 0])
        },
        'temp': [922]
    }
}

for mat in mats:
    for t in mats[mat]['temp']:
        temp = str(t)
        mats[mat][temp]['DIFFCOEF'] = 1 / 3 / (mats[mat][temp]['sigma_a'] +
                                         mats[mat][temp]['sigma_tr'])
        mats[mat][temp]['REMXS'] = mats[mat][temp]['sigma_a']
        out_scatter = np.zeros(groups)
        for i in range(groups):
            for j in range(groups):
                if i != j: # scatter from i to j
                    out_scatter[i] += \
                        mats[mat][temp]['GTRANSFXS'][i * groups + j]
        mats[mat][temp]['REMXS'] += out_scatter
        if 0 in mats[mat][temp]['FISSXS']:
            mats[mat][temp]['FISSE'] = np.zeros(groups)
        else:
            mats[mat][temp]['FISSE'] = mats[mat][temp]['kappa_fission'] / \
                mats[mat][temp]['FISSXS'] / 1.602e-13 # MeV
        mats[mat][temp].pop('sigma_tr')
        mats[mat][temp].pop('sigma_a')
        mats[mat][temp].pop('kappa_fission')
        for data in mats[mat][temp]:
            mats[mat][temp][data] = mats[mat][temp][data].tolist()

with open('xsdata.json', 'w') as f:
    json.dump(mats, f, sort_keys=True, indent=4)