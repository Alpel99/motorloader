import matplotlib.pyplot as plt
import numpy as np

MAX_VALUES = 5

steps = [1000, 1000, 1000, 1000, 10000]*5
print(steps)


f = lambda x: 0.1/(5*MAX_VALUES)*x
power = [f(x) for x in range(1,MAX_VALUES*5+1)]

res = [None]*MAX_VALUES*5*2
res[::2] = power
res[1::2] = steps

plt.plot(np.cumsum(res[1::2]), res[::2])
plt.show()
print(res)

np.savetxt("res_disc.txt", res, delimiter=",", newline=",", fmt="%.3f")