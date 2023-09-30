import requests
import numpy as np
import cv2
from enum import Enum
import time

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

class MotorsDirection(str, Enum):
    UP_RIGHT   = "up_right"
    RIGHT      = "right"
    DOWN_RIGHT = "down_right"
    DOWN_LEFT  = "down_left"
    LEFT       = "left"
    UP_LEFT    = "up_left"

def http_request(http_address):
    print(f"request: '{http_address}'",flush=True)
    response = requests.get(http_address)
    print(f"response:",flush=True)
    print(f"   status: {response.status_code}",flush=True)
    print(f"   content-type: {response.headers['content-type']}",flush=True)
    return response

def camera_capture():
    global iteration

    img_path = f"camera_capture_{iteration}.png"
    iteration = iteration+1

    if simu:
        print(f"read '{img_path}' from disk",flush=True)
        return cv2.imread(img_path,cv2.IMREAD_GRAYSCALE)

    response = http_request(f"{esp32_http_address}/capture")
    byte_array = bytearray(response.content)
    array = np.asarray(byte_array, dtype=np.uint8)
    img = cv2.imdecode(array, cv2.IMREAD_COLOR)

    print(f"write '{img_path}' to disk",flush=True)

    return img

def motors_move_one_step(direction):
    print(f"motors_move_one_step({direction})",flush=True)
    if not simu:
        response = http_request(f"{esp32_http_address}/cmd={direction}&continuous=0")
        # todo : treat errors

def get_motors_status():
    print("get_motors_status",flush=True)
    if not simu:
        response = http_request(f"{esp32_http_address}/status")
        print(f"   json: {response.json}",flush=True)
        # todo : treat errors
        # todo : return motors status


