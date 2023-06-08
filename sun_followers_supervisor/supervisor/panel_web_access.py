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


simu=SimuMode.RECORD
#simu=SimuMode.REPLAY

#replay_folder = "./solar_concentrator_manip_20230604/capture-20230604-a/"
replay_folder = "capture-20230602-c-plafond"
record_folder = "capture"
record_prefix = "camera_capture"

print(f"simu: '{simu}'",flush=True)

iteration=0

esp32_http_address = "http://192.168.209.101"

# Last full color capture is cached to be reused by cameraCaptureAndReplaceArea, it's :
# - color
# - not blured
# - without time footer
last_full_color_img = None

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

# Capture and return the given area in grayscale
def cameraCaptureArea(area):
    response = httpRequest(f"{esp32_http_address}/capture_area?left_px={area.left_px}&top_px={area.top_px}&right_px={area.right_px}&bottom_px={area.bottom_px}")
    byte_array = bytearray(response.content)

    height = area.bottom_px - area.top_px + 1
    width = area.right_px - area.left_px + 1
    img = np.zeros((height, width), dtype=np.uint8)
    img = np.reshape(byte_array, (height, width))

    return img

# Recapture only the given area and replace it in the last full captured image
# Return gray image at full resolution
def cameraCaptureAndReplaceArea(area):
    if last_full_color_img is None or simu==SimuMode.REPLAY:
        return cameraCapture()

    global iteration
    iteration = iteration + 1

    area_img = cameraCaptureArea(area)

    img = cv2.cvtColor(last_full_color_img, cv2.COLOR_BGR2GRAY)
    img[area.top_px:area.bottom_px+1, area.left_px:area.right_px+1] = area_img
    addDate(img)

    if simu==SimuMode.RECORD:
        img_path = f"capture/camera_capture_{iteration}.png"
        cv2.imwrite(img_path,img)
        print(f"cameraCapture: write '{img_path}' to disk",flush=True)

    img = cv2.blur(img,(3,3))
    return img

# Capture full image
# Save color image if saved_filename_prefix is not None
# Return gray image at full resolution
def cameraCapture(saved_filename_prefix = None):
    global iteration,last_full_color_img
    iteration = iteration + 1

    if simu==SimuMode.REPLAY:
        img_path = f"{replay_folder}/camera_capture_{iteration}.png"
        if not os.path.isfile(img_path):
            print(f"file '{img_path}' does not exist: exit application",flush=True)
            exit(1)

        print(f"cameraCapture: read '{img_path}' from disk",flush=True)
        gray_img = cv2.imread(img_path,cv2.IMREAD_GRAYSCALE)
        return cv2.blur(gray_img,(3,3))

    response = httpRequest(f"{esp32_http_address}/capture")
    byte_array = bytearray(response.content)
    array = np.asarray(byte_array, dtype=np.uint8)
    color_img = cv2.imdecode(array, cv2.IMREAD_COLOR)

    last_full_color_img = color_img.copy()
    capture_time = datetime.now().astimezone(tz.gettz('Europe/Paris'))
    time_suffix = capture_time.strftime("-%Y%m%d-%H%M%S")

    addDate(color_img)
    if simu==SimuMode.RECORD:
        img_path = f"{record_folder}/{record_prefix}_{iteration}.png"
        cv2.imwrite(img_path,color_img)
        print(f"cameraCapture: write '{img_path}' to disk",flush=True)
    if saved_filename_prefix is not None:
        img_path = f"{record_folder}/{saved_filename_prefix}{time_suffix}.png"
        cv2.imwrite(img_path,color_img)
        print(f"cameraCapture: write '{img_path}' to disk",flush=True)

    img = cv2.cvtColor(color_img, cv2.COLOR_BGR2GRAY)
    img = cv2.blur(img,(3,3))
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

