import numpy as np


#serial definitions
port = 'COM3'
baudrate = 115200

#Data as the program handles
SHperiod = np.uint32(20)
ICGperiod = np.uint32(500000)
AVGn = np.uint8([0,1])

#Data arrays for received bytes
rxData8 = np.zeros(7388, np.uint8)
rxData16 = np.zeros(3694, np.uint16)
rxData128 = np.zeros(7388, np.uint32)


rxDataVC = np.zeros(4096, np.uint16)

#Arrays for data to transmit
txsh = np.uint8([0,0,0,0]) 
txicg = np.uint8([0,0,0,0])
