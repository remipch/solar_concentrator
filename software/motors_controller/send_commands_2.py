# Copyright (C) 2024 Rémi Peuchot (https://remipch.github.io/)
# This code is distributed under GNU GPL v3 license

import getch

import serial

motors = serial.Serial('/dev/ttyUSB0',baudrate=19200)

panel = 0

def writeCommand(info, command):
    print(info,flush=True)
    res = motors.write((command + "\n").encode())
    print(str(res) + " byte(s) written")

def writeCommandToCurrentPanel(info, command_panel_0, command_panel_1):
    if panel==0:
        writeCommand(info, command_panel_0)
    else:
        writeCommand(info, command_panel_1)

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
    if ord(ch)==27:  # escape
        exit()
    elif ch == 'p':
        panel = 1-panel
        print("Switch to panel: ", panel, flush=True)
    elif ch == 's':
        writeCommand("Status", "s")
    elif ch == 'm':
        writeCommand("Measure buffer", "m:100")
    elif ch == '0':
        writeCommand("Off", "o:0,1000")
    elif ch == '8':
        writeCommandToCurrentPanel("Step UP", "o:5,50,100" ,"o:80,150,200")
    elif ch == '9':
        writeCommandToCurrentPanel("Step UP_RIGHT", "o:1,50,50", "o:16,150,200")
    elif ch == '6':
        writeCommandToCurrentPanel("Step RIGHT", "o:9,50,50", "o:144,150,200")
    elif ch == '3':
        writeCommandToCurrentPanel("Step DOWN_RIGHT", "o:8,50,100" ,"o:128,150,200")
    elif ch == '2':
        writeCommandToCurrentPanel("Step DOWN", "o:10,50,100" ,"o:160,150,200")
    elif ch == '1':
        writeCommandToCurrentPanel("Step DOWN_LEFT", "o:2,50,100" ,"o:32,150,200")
    elif ch == '4':
        writeCommandToCurrentPanel("Step LEFT", "o:6,50,100" ,"o:96,150,200")
    elif ch == '7':
        writeCommandToCurrentPanel("Step UP_LEFT", "o:4,50,100" ,"o:64,150,200")

    else:
        print("unkown key")
        continue
    readResult()

