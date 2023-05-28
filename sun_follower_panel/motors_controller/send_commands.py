import getch

import serial

motors = serial.Serial('/dev/ttyUSB0',baudrate=19200)

def writeCommand(info, command):
    print(info,flush=True)
    res = motors.write((command + "\n").encode())
    print(str(res) + " byte(s) written")

def readResult():
    res = motors.read(1000).decode("utf-8")
    print(res)

# read the initial text with bigger timeout
motors.timeout = 2
readResult()
motors.timeout = 0.5

print("Waiting for direction digit keys",flush=True)

while True:
    ch = getch.getch()
    if ord(ch)==27: # escape
        exit()
    elif ch == 's':
        writeCommand("Status", "s")
    elif ch == 'r':
        writeCommand("Release all", "o:21,1000")
    elif ch == 't':
        writeCommand("Tense all", "o:42,1000,80")
    elif ch == '8':
        writeCommand("UP", "o:20,500;o:2,5000,30")
    elif ch == '9':
        #writeCommand("UP_RIGHT", "o:16,1000;o:2,5000,30")
        writeCommand("UP_RIGHT", "o:16,500,30;o:2,1500,30")
    elif ch == '6':
        #writeCommand("RIGHT", "o:16,200;o:8,5000,25")
        writeCommand("RIGHT", "o:16,500,30;o:8,1500,30")
    elif ch == '3':
        #writeCommand("DOWN_RIGHT", "o:17,500;o:16,500;o:42,5000,80")
        writeCommand("DOWN_RIGHT", "o:1,500,30;o:8,1500,30")
    elif ch == '2':
        writeCommand("DOWN", "o:1,200;o:40,5000,50")
    elif ch == '1':
        #writeCommand("DOWN_LEFT", "o:5,500;o:42,5000,80")
        writeCommand("DOWN_LEFT", "o:1,200,30;o:32,1500,30")
    elif ch == '4':
        #writeCommand("LEFT", "o:4,200;o:32,5000,20")
        writeCommand("LEFT", "o:4,500,30;o:32,1500,30")
    elif ch == '7':
        #writeCommand("UP_LEFT", "o:4,1000;o:2,5000,25")
        writeCommand("UP_LEFT", "o:4,500,30;o:2,1500,30")
    else:
        print("unkown key")
        continue
    readResult()

