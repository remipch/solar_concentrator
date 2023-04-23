import requests
import numpy as np
import cv2
from enum import Enum
from motors_direction import MotorsDirection
import os.path


class MotorsStatus(str, Enum):
    ERROR           = "ERROR"
    UNINITIALIZED   = "UNINITIALIZED"
    IDLE            = "IDLE"
    MOVING          = "MOVING"
    MOVING_ONE_STEP = "MOVING_ONE_STEP"
    TIGHTENING      = "TIGHTENING"
    LOCKED          = "LOCKED"



# if True : read images from disk
# if False : capture image from esp32 camera
simu=True

if simu:
    iteration=0
else:
    iteration=100

# does not work with esp32 network name : http://esp32_test.local/capture
# network tool avahi-discover does not work from the container
esp32_http_address = "http://192.168.76.180"

def httpRequest(http_address):
    print(f"request: '{http_address}'",flush=True)
    response = requests.get(http_address)
    print(f"response:",flush=True)
    print(f"   status: {response.status_code}",flush=True)
    print(f"   content-type: {response.headers['content-type']}",flush=True)
    return response

def cameraCapture():
    global iteration

    img_path = f"camera_capture_{iteration}.png"
    iteration = iteration+1

    if simu:
        if not os.path.isfile(img_path):
            print(f"file '{img_path}' does not exist: exit application",flush=True)
            exit(1)

        print(f"read '{img_path}' from disk",flush=True)
        return cv2.imread(img_path,cv2.IMREAD_GRAYSCALE)

    response = httpRequest(f"{esp32_http_address}/capture")
    byte_array = bytearray(response.content)
    array = np.asarray(byte_array, dtype=np.uint8)
    img = cv2.imdecode(array, cv2.IMREAD_COLOR)

    print(f"write '{img_path}' to disk",flush=True)

    return img

def moveOneStep(direction):
    print(f"motorsMoveOneStep({direction})",flush=True)
    if not simu:
        response = httpRequest(f"{esp32_http_address}/cmd={direction}&continuous=0")
        # todo : treat errors

def getMotorsStatus():
    print("getMotorsStatus",flush=True)
    if not simu:
        response = httpRequest(f"{esp32_http_address}/status")
        print(f"   json: {response.json}",flush=True)
        # todo : treat errors
        # todo : return motors status
    return MotorsStatus.LOCKED

