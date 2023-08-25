import matplotlib.pyplot as plt
import numpy as np

MAX_VALUES = 1200

f = lambda x: abs(0.05*np.sin(x*4*np.pi/MAX_VALUES))+0.05

res = [f(x) for x in range(0,MAX_VALUES)]

plt.plot(res)
plt.show()
print(res)

np.savetxt("res.txt", res, delimiter=",", newline=",", fmt="%.3f")