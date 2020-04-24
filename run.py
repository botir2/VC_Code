import time
import serial
import sys
import configs
import mmap
from struct import *
import numpy as np
from pyqtgraph.Qt import QtGui, QtCore
import pyqtgraph as pg
from threading import Thread


#QtGui.QApplication.setGraphicsSystem('raster')
app = QtGui.QApplication([])



def Connection(serial_port):
    try:
        serial_port = serial.Serial(configs.port, configs.baudrate, timeout = None)
        print(configs.port, 'connected')
        #print(cofig.port, 'cofig.port')

    except serial.SerialException:
        print('SerialException')

    if not serial_port:
        raise serial.SerialException('not found, but tried hard.')

    return serial_port

def main():
    serial_port = False
    ser = Connection(serial_port)

    ser.write(str.encode("#CSDTP:1%"))



    # split 32-bit integers to be sent into 8-bit data
    configs.txsh[0] = (configs.SHperiod >> 24) & 0xff
    configs.txsh[1] = (configs.SHperiod >> 16) & 0xff
    configs.txsh[2] = (configs.SHperiod >> 8) & 0xff
    configs.txsh[3] = configs.SHperiod & 0xff

    configs.txicg[0] = (configs.ICGperiod >> 24) & 0xff
    configs.txicg[1] = (configs.ICGperiod >> 16) & 0xff
    configs.txicg[2] = (configs.ICGperiod >> 8) & 0xff
    configs.txicg[3] = configs.ICGperiod & 0xff

    # Transmit SH-period
    #ser.write(configs.txsh)
    # Transmit last of the 12 bytes required to start integration
    #ser.write(configs.AVGn)

    ####################################### Recieving CCD data #################################################

    while True:
        while ser.inWaiting() > 0:
            configs.rxData8 = ser.read(7332)
            #datas = ser.read(100)
            #datas = unpack("q", par)
            #print(datas)
            for rxi in range(128):
                configs.rxDataVC[rxi] = (configs.rxData8[2 * rxi + 1] << 8) + configs.rxData8[2 * rxi]
                #print(configs.rxDataVC)
                global naparrays
                naparrays = configs.rxDataVC
                #print(naparrays[0,])

            #print('Before:\n{}'.format(par.rstrip()))
            #print(int.from_bytes(par[0xFFF0], byteorder='little'))
            #print(int.from_bytes([255, 255, 0], byteorder='big'))
            time.sleep(.01)



    ser.close()

def datas(data):
    return data

class Plotting(Thread):
    def __init__(self):
        super(Plotting, self).__init__()
    def run(self):
       main()

if __name__ == '__main__':
    #th = Plotting()
    #th.setDaemon(True)
    #th.start()
    win = pg.GraphicsWindow(title="Basic plotting examples")  # PyQtGraph grahical window
    win.resize(1500, 750)
    win.setWindowTitle('pyqtgraph example: Plotting')  # Title of python window

    # Updating Plot
    p6 = win.addPlot(title="CCD 50Hz plot")
    curve = p6.plot(pen='y')
    data = np.random.normal(size=(5000, 3648))
    ptr = 5000


    def update():
        global curve, data, ptr, p6
        curve.setData(data[ptr % 3648])
        print("____________________________")
        print(data[ptr % 2])
        if ptr == 0:
            p6.enableAutoRange('xy', False)  ## stop auto-scaling after the first data set is plotted
        ptr += 1


    timer = QtCore.QTimer()
    timer.timeout.connect(update)
    timer.start(.01)
    QtGui.QApplication.instance().exec_()