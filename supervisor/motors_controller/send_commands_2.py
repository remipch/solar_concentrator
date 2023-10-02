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
    elif ch == 'm':
        writeCommand("Measure buffer", "m:100")
    elif ch == 'a':
        writeCommand("Unroll all", "o:21,1000,80")
    elif ch == 'A':
        writeCommand("Roll all", "o:42,1000,80")
    elif ch == 'u':
        writeCommand("Unroll up", "o:1,1000,30")
    elif ch == 'U':
        writeCommand("Roll up", "o:2,5000,30")
    elif ch == 'j':
        writeCommand("Unroll down right", "o:4,100,30")
    elif ch == 'J':
        writeCommand("Roll down right", "o:8,500,30")
    elif ch == 'h':
        writeCommand("Unroll down left", "o:16,100,30")
    elif ch == 'H':
        writeCommand("Roll down left", "o:32,500,30")
    elif ch == '0':
        writeCommand("Off", "o:0,1000")
    elif ch == '8':
        writeCommand("Step UP", "o:20,50,30")
    elif ch == '9':
        writeCommand("Step UP_RIGHT", "o:16,80,30")
    elif ch == '6':
        writeCommand("Step RIGHT", "o:24,80,80")
    elif ch == '3':
        writeCommand("Step DOWN_RIGHT", "o:8,100,80")
    elif ch == '2':
        writeCommand("Step DOWN", "o:40,100,80")
    elif ch == '1':
        writeCommand("Step DOWN_LEFT", "o:32,100,80")
    elif ch == '4':
        writeCommand("Step LEFT", "o:36,80,80")
    elif ch == '7':
        writeCommand("Step UP_LEFT", "o:4,80,80")
    elif ch == 't':
        writeCommand("Step UP", "o:20,500,30")
    elif ch == 'g':
        writeCommand("Step RIGHT", "o:24,750,80")
    elif ch == 'v':
        writeCommand("Step DOWN", "o:40,1000,80")
    elif ch == 'f':
        writeCommand("Step LEFT", "o:36,750,80")
    elif ch == 'T':
        writeCommand("Step UP", "o:20,2500,30")
    elif ch == 'G':
        writeCommand("Step RIGHT", "o:24,3750,80")
    elif ch == 'V':
        writeCommand("Step DOWN", "o:40,5000,80")
    elif ch == 'F':
        writeCommand("Step LEFT", "o:36,3750,80")
    else:
        print("unkown key")
        continue
    readResult()

