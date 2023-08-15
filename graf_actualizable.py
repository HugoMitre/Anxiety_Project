from collections import deque
import matplotlib.pyplot as plt
import numpy as np

datos = deque(maxlen = 10)

for i in np.sin(range(100)):
    datos.append(i)
    print(datos)
    plt.clf()
    plt.plot(datos)
    plt.pause(0.1)