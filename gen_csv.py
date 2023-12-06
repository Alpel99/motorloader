import matplotlib.pyplot as plt
import numpy as np

MAX_VALUES = 600

f = lambda x: abs(0.05*np.sin(x*4*np.pi/MAX_VALUES))+0.05

res = [f(x) for x in range(0,MAX_VALUES)]

# plt.plot(res)
# plt.show()

np.savetxt("res.txt", res, delimiter=",", newline=",", fmt="%.3f")

def genSinCurve(steps, height, dist):
    data = []
    for i in range(1,steps):
        r = np.sin(np.pi*(i/steps))*height
        data.append(r)
        data.append(dist)
    return data

r = []
for h in [0.1,0.2,0.3,0.4]:
    for d in [50, 100, 200, 400, 800, 1500]:
        r += genSinCurve(50, h, d)

print(len(r))

plt.plot(np.cumsum(r[1::2]), r[::2])
plt.show()

r1 = np.array(r).reshape(-1, 2)
print(r1.shape)
np.savetxt("res_sin.txt", r1, delimiter=",", newline=",", fmt=["%.4f","%d"])