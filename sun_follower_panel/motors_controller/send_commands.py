import getch

import serial

motors = serial.Serial('/dev/ttyUSB0',baudrate=19200,timeout=2)

def writeCommand(info, command):
    print(info,flush=True)
    res = motors.write(command.encode())
    print(res)

while True:
    ch = getch.getch()
    if ord(ch)==27: # escape
        exit()
    elif ch == '8':
        writeCommand("UP", "o:20,500;o:2,5000,30")
    elif ch == '9':
        writeCommand("UP_RIGHT", "o:16,1000;o:2,5000,30")
    elif ch == '6':
        writeCommand("RIGHT", "o:16,500;o:8,5000,25")
    elif ch == '3':
        writeCommand("DOWN_RIGHT", "o:17,500;o:16,500;o:42,5000,80")
    elif ch == '2':
        writeCommand("DOWN", "o:1,500;o:40,5000,50")
    elif ch == '1':
        writeCommand("DOWN_LEFT", "o:5,500;o:42,5000,80")
    elif ch == '4':
        writeCommand("LEFT", "o:4,500;o:42,5000,80")
    elif ch == '7':
        writeCommand("UP_LEFT", "o:4,1000;o:2,5000,25")

