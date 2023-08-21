import matplotlib.pyplot as plt
import numpy as np

MAX_VALUES = 1200

f = lambda x: abs(0.5*np.sin(x*2*np.pi/MAX_VALUES))+0.5

res = [f(x) for x in range(0,MAX_VALUES)]

plt.plot(res)
# plt.show()
print(res)

np.savetxt("res.txt", res, delimiter=",", newline=",", fmt="%.3f")