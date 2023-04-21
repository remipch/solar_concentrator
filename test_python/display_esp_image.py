import requests
import numpy as np
import cv2

cv2.namedWindow("camera", cv2.WINDOW_NORMAL)

while True:

    # does not work with esp32 network name : http://esp32_test.local/capture
    # network tool avahi-discover does not work from the container
    response = requests.get("http://192.168.76.180/capture")
    print(f"status: {response.status_code} : content-type: {response.headers['content-type']}")
    byte_array = bytearray(response.content)
    array = np.asarray(byte_array, dtype=np.uint8)
    image = cv2.imdecode(array, cv2.IMREAD_COLOR)

    cv2.imshow("camera", image)
    key = cv2.waitKey(1)
    print(f"key={key}",flush=True)

    # exit on escape
    if key==27:
        exit(0)

