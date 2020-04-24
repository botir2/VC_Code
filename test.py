import time
import serial
import sys
import config
import numpy as np
from struct import *
import csv
import pandas as pd


def Connection(serial_port):
    try:
        port = 'COM3'
        serial_port = serial.Serial(port, 115200, timeout = None)
        print(port, 'connected')
        #print(cofig.port, 'cofig.port')

    except serial.SerialException:
        print('SerialException')

    if not serial_port:
        raise serial.SerialException('not found, but tried hard.')

    return serial_port

def writedatas(datas):
    with open('CCD.csv', mode='w') as employee_file:
        employee_writer = csv.writer(employee_file, delimiter=',', quotechar='"', quoting=csv.QUOTE_MINIMAL)
        employee_writer.writecol(datas)

def main():
    serial_port = False
    ser = Connection(serial_port)

    ser.write(str.encode("#CSDTP:1%"))

    # serial_port.write(str.encode("@a0080#@"))

    ########################################################
    #
    #       data process
    #
    #
    global BytesRead
    total_bytes = 0
    total_num = 0

    global ftStatus
    while True:
        dataarr = {}
        loop_sign = 0;
        #df = pd.DataFrame(columns=['CCD'])
        while loop_sign <= 3648:
                ftStatus = ser.read(2)
                total_bytes = unpack('!BB', ftStatus)
                #data['0':'3648']
                loop_sign = loop_sign + 1
                #frame = total_bytes[0] * 256
                #print(loop_sign, "   ", total_bytes[0] * 256)
                arr = np.array([total_bytes[0] * 256])
                print(np.append(arr, ))
                #print(arr)
                #print(arr)
                #print(total_bytes[0] * 256)
                #total_num = total_num + 1;
                #print((total_bytes[0] * 256))
                #for pix_number in range(1,3648):
                    #print(pix_number, "=",)
                    #print((total_bytes[0] * 256))
                #print(total_bytes[1:])


    #print(arr)
    ser.close()

if __name__ == '__main__':
    main()
