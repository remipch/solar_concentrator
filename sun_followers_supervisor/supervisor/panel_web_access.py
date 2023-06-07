import requests
import numpy as np
import cv2
from enum import Enum
import os.path
from datetime import datetime
from dateutil import tz
import time

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


#simu=SimuMode.RECORD
simu=SimuMode.REPLAY

#replay_folder = "./solar_concentrator_manip_20230604/capture-20230604-a/"
#replay_folder = "."
replay_folder = "capture-20230602-c-plafond/"

print(f"simu: '{simu}'",flush=True)

iteration=0

esp32_http_address = "http://192.168.209.101"

#TODO : remove and replace by cameraCaptureColor
last_color_capture_img = None
last_color_capture_time = None

def httpRequest(http_address):
    for retry in range(20):
        print(f"    httpRequest: {http_address} (try {retry+1})",flush=True)
        try:
            response = requests.get(http_address,timeout=(10,10))
            if response.status_code != 200:
                print(f"    response:",flush=True)
                print(f"      status: {response.status_code}",flush=True)
                print(f"      content-type: {response.headers['content-type']}",flush=True)
                raise Exception(f"Incorrect http status")
            return response
        except Exception as inst:
            print(f"    Http request exception:",flush=True)
            print(type(inst))
            print(inst.args)
            time.sleep(3)
            continue
    raise Exception(f"Fail http request after multiple tentatives")

def addDate(img):
    time = datetime.now().astimezone(tz.gettz('Europe/Paris'))
    text = time.strftime("%Y-%m-%d %H:%M:%S")
    height = img.shape[0]
    cv2.putText(img, text, (10, height - 10), cv2.FONT_HERSHEY_SIMPLEX, 1, (255,255,255), lineType = cv2.LINE_AA)

def cameraCaptureArea(left_px, top_px, right_px, bottom_px):
    global iteration
    iteration = iteration + 1

    if simu==SimuMode.REPLAY:
        raise Exception("not implemented")

    response = httpRequest(f"{esp32_http_address}/capture_area?left_px={left_px}&top_px={top_px}&right_px={right_px}&bottom_px={bottom_px}")
    byte_array = bytearray(response.content)

    height = bottom_px - top_px + 1
    width = right_px - left_px + 1
    img = np.zeros((height, width), dtype=np.uint8)
    img = np.reshape(byte_array, (height, width))

    if simu==SimuMode.RECORD:
        img_path = f"camera_capture_{iteration}.png"
        cv2.imwrite(img_path,img)
        print(f"cameraCapture: write '{img_path}' to disk",flush=True)

    img = cv2.blur(img,(3,3))
    return img

def cameraCapture():
    global iteration,last_color_capture_img,last_color_capture_time
    iteration = iteration + 1

    if simu==SimuMode.REPLAY:
        img_path = f"{replay_folder}/camera_capture_{iteration}.png"
        if not os.path.isfile(img_path):
            print(f"file '{img_path}' does not exist: exit application",flush=True)
            exit(1)

        print(f"cameraCapture: read '{img_path}' from disk",flush=True)
        return cv2.imread(img_path,cv2.IMREAD_GRAYSCALE)

    img_path = f"camera_capture_{iteration}.png"

    response = httpRequest(f"{esp32_http_address}/capture")
    byte_array = bytearray(response.content)
    array = np.asarray(byte_array, dtype=np.uint8)
    last_color_capture_img = cv2.imdecode(array, cv2.IMREAD_COLOR)
    last_color_capture_time = datetime.now().astimezone(tz.gettz('Europe/Paris'))
    if simu==SimuMode.RECORD:
        addDate(last_color_capture_img)
        cv2.imwrite(img_path,last_color_capture_img)
        print(f"cameraCapture: write '{img_path}' to disk",flush=True)

    img = cv2.imdecode(array, cv2.IMREAD_GRAYSCALE)
    img = cv2.blur(img,(3,3))
    addDate(img)
    return img

#TODO : remove and replace by cameraCaptureColor
def saveLastColorCapture(img_path_prefix):
    if last_color_capture_img is not None:
        suffix = last_color_capture_time.strftime("-%Y%m%d-%H%M%S.png")
        img_path = img_path_prefix+suffix
        cv2.imwrite(img_path,last_color_capture_img)
        print(f"saveLastColorCapture: write '{img_path}' to disk",flush=True)

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

