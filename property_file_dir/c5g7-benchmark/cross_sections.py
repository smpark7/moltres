import numpy as np

# absorption
# transport
# nu
# fission
# diffcoef = 1 / (3 * (absorption + transport))
# nsf = nu * fission
# removal = absorption + outscatter

absorption = np.array([8.02480e-3, 3.71740e-3, 2.67690e-2, 9.6236e-2,
                       3.00200e-2, 1.11260e-1, 2.82780e-1])
transport = np.array([1.77949e-1, 3.29805e-1, 4.80388e-1, 5.54367e-1,
                      3.11801e-1, 3.95168e-1, 5.64406e-1])
nu = np.array([2.78145, 2.47443, 2.43383, 2.43380, 2.43380, 2.43380, 2.43380])
fission = np.array([7.21206e-3, 8.19301e-4, 6.45320e-3, 1.85648e-2,
                    1.78084e-2, 8.30348e-2, 2.16004e-1])

diffcoef = 1 / (3 * absorption + transport)