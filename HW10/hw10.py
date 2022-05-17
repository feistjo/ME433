import time
from ulab import numpy as np

a = np.empty(1024)

sin_freq_1 = 1
sin_freq_2 = 10
sin_freq_3 = 100

for i in range(0, 1024):
    a[i] = np.sin(sin_freq_1*i/(1024/2*np.pi)) + np.sin(sin_freq_2*i/(1024/2*np.pi)) + np.sin(sin_freq_3*i/(1024/2*np.pi))

fft = np.fft.fft(a)

for i in fft[0]:
    print("("+str(i)+",)")
    time.sleep(0.005)
