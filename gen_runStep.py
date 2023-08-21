import matplotlib.pyplot as plt
import numpy as np

MAX_VALUES = 120

f = lambda x: abs(0.5*np.sin(x*2*np.pi/MAX_VALUES))+0.5
power = [f(x) for x in range(0,MAX_VALUES)]

s = lambda x: (x**2)*50+50
steps = [s(x) for x in np.linspace(-2,1,MAX_VALUES)]
plt.plot(steps)
plt.show()

res = [None]*MAX_VALUES*2
res[::2] = power
res[1::2] = steps

plt.plot(np.cumsum(res[1::2]), res[::2])
plt.show()
print(res)

np.savetxt("res_step.txt", res, delimiter=",", newline=",", fmt="%.3f")