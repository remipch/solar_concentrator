import requests
import numpy as np
import cv2
from enum import Enum
import os.path
import subprocess

class MotorsStatus(str, Enum):
    ERROR           = "ERROR"
    UNINITIALIZED   = "UNINITIALIZED"
    IDLE            = "IDLE"
    MOVING          = "MOVING"
    MOVING_ONE_STEP = "MOVING_ONE_STEP"
    TIGHTENING      = "TIGHTENING"
    LOCKED          = "LOCKED"



class SimuMode(str, Enum):
    NONE     = "NONE"
    RECORD   = "RECORD"
    REPLAY   = "REPLAY"


simu=SimuMode.NONE

print(f"simu: '{simu}'",flush=True)

if simu==SimuMode.RECORD:
    iteration=100
else:
    iteration=0

def resolve_host_name(host_name):
    output = subprocess.check_output(["avahi-resolve-host-name", host_name], text=True)
    ip_address = output.split()[1]
    return ip_address

# does not work with esp32 network name directly : http://esp32_test.local/capture
# because name resolution does not work from the container (ping does not works from the container with hostname)
# because it requires 'service dbus start' command to be executed in container
# but then it makes dbus fail from the host ('systemctl' and other system commands fails)
# so we call "avahi-resolve-host-name" command manually
# which requires dbus anyway (with the volume mapping "/var/run/dbus:/var/run/dbus" in docker-compose.yml)
# (WTF... I don't know how to fix this mess correctly)
esp32_http_address = ""

if simu!=SimuMode.REPLAY:
    esp32_http_address = "http://" + resolve_host_name("esp32_test.local")

def httpRequest(http_address):
    response = requests.get(http_address)
    if response.status_code != 200:
        print(f"  httpRequest: '{http_address}'",flush=True)
        print(f"    response:",flush=True)
        print(f"      status: {response.status_code}",flush=True)
        print(f"      content-type: {response.headers['content-type']}",flush=True)
        raise Exception(f"Incorrect http status")
    return response

def cameraCapture():
    global iteration

    img_path = f"camera_capture_{iteration}.png"

    iteration = iteration + 1

    if simu==SimuMode.REPLAY:
        iteration = iteration % 10
        if not os.path.isfile(img_path):
            print(f"file '{img_path}' does not exist: exit application",flush=True)
            exit(1)

        print(f"cameraCapture: read '{img_path}' from disk",flush=True)
        return cv2.imread(img_path,cv2.IMREAD_GRAYSCALE)

    response = httpRequest(f"{esp32_http_address}/capture")
    byte_array = bytearray(response.content)
    array = np.asarray(byte_array, dtype=np.uint8)
    img = cv2.imdecode(array, cv2.IMREAD_GRAYSCALE)
    if simu==SimuMode.RECORD:
        cv2.imwrite(img_path,img)
        print(f"cameraCapture: write '{img_path}' to disk",flush=True)
    return img

def moveOneStep(direction):
    print(f"moveOneStep: {direction}",flush=True)
    if not simu==SimuMode.REPLAY:
        response = httpRequest(f"{esp32_http_address}/motors_command?cmd={direction}&continuous=0")

def stopMove():
    print("stopMove",flush=True)
    if not simu==SimuMode.REPLAY:
        response = httpRequest(f"{esp32_http_address}/motors_command?cmd=stop&continuous=0")

def getMotorsStatus():
    if simu==SimuMode.REPLAY:
        status = MotorsStatus.LOCKED
    else:
        response = httpRequest(f"{esp32_http_address}/motors_status")
        #print(f"   json: {response.json()}",flush=True)
        status = MotorsStatus(response.json()["motors-state"])
    print(f"getMotorsStatus: {status}",flush=True)
    return status

