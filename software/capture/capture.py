import cv2
from datetime import datetime
from dateutil import tz
import time
from urllib.request import urlopen
import numpy as np
import argparse

WEBCAM_PREFIX = "webcam/webcam"
ESPCAM_PREFIX = "espcam/espcam"

ESPCAM_URL = r'http://192.168.1.10/image'

# Return time in Paris (TODO: use locale config ?)
def getCurrentTime():
    return datetime.now().astimezone(tz.gettz('Europe/Paris'))

def getFileTimeSuffix(time):
    return time.strftime("-%Y%m%d-%H%M%S")

def addDate(img,time):
    text = time.strftime("%Y-%m-%d %H:%M:%S")
    height = img.shape[0]
    cv2.putText(img, text, (10, height - 10), cv2.FONT_HERSHEY_SIMPLEX, 1, (0,0,0), 3, lineType = cv2.LINE_AA)
    cv2.putText(img, text, (10, height - 10), cv2.FONT_HERSHEY_SIMPLEX, 1, (255,255,255), lineType = cv2.LINE_AA)

arg_parser = argparse.ArgumentParser(description='Show and capture espcam and webcam')
arg_parser.add_argument('-e', dest='espcam', action='store_true', help='record espcam')
arg_parser.add_argument('-w', dest='webcam', action='store_true', help='record webcam')
arg_parser.add_argument('-s', '--sleep_ms', type=int, default=1000, help='sleep time (ms) between captures')

args = arg_parser.parse_args()

print(f"Espcam enabled : {args.espcam}")
print(f"Webcam enabled : {args.webcam}")
print(f"sleep_ms : {args.sleep_ms}")

if not args.espcam and not args.webcam:
    exit(0)

if args.webcam:
    webcam = cv2.VideoCapture(2)  # Use 0 for the default camera, 2 for USB webcam
    webcam.set(cv2.CAP_PROP_BUFFERSIZE, 1)
    _, _ = webcam.read() # WTF we need to capture a first frame before setting exposure...
    webcam.set(cv2.CAP_PROP_EXPOSURE, 1)
    webcam.set(cv2.CAP_PROP_FRAME_WIDTH, 1920)
    webcam.set(cv2.CAP_PROP_FRAME_HEIGHT, 1080)

    # Check if the webcam is opened correctly
    if not webcam.isOpened():
        raise IOError("Cannot open webcam")

while True:
    time = getCurrentTime()
    time_suffix = getFileTimeSuffix(time)

    if args.espcam:
        img_resp = urlopen(ESPCAM_URL)
        imgnp = np.asarray(bytearray(img_resp.read()), dtype="uint8")
        espcam_frame = cv2.imdecode(imgnp, -1)

        addDate(espcam_frame,time)
        cv2.imshow("espcam", espcam_frame)

        espcam_path = f"{ESPCAM_PREFIX}{time_suffix}.jpg"
        cv2.imwrite(espcam_path,espcam_frame)

    if args.webcam:
        ret, webcam_frame = webcam.read()
        if not ret:
            continue

        addDate(webcam_frame,time)

        scaled_frame = cv2.resize(webcam_frame, None, fx=0.5, fy=0.5, interpolation=cv2.INTER_LINEAR)
        cv2.imshow('webcam', scaled_frame)

        webcam_path = f"{WEBCAM_PREFIX}{time_suffix}.jpg"
        cv2.imwrite(webcam_path,webcam_frame)

    # Break the loop on pressing 'q' key
    if cv2.waitKey(args.sleep_ms) & 0xFF == ord('q'):
        break

if args.webcam:
    webcam.release()

cv2.destroyAllWindows()


