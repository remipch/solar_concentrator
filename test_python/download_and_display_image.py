import requests
import numpy as np
import cv2

BASE_URL = 'https://fullhdwall.com/'

response = requests.get(f"{BASE_URL}/wp-content/uploads/2016/04/Yellow-Sun-Picture.jpeg")
print(type(response))

print(response.status_code)
print(response.headers['content-type'])
print(response.headers)
print(response.encoding)
print(type(response.text))

buffer = response.content

byte_array = bytearray(buffer)
array = np.asarray(byte_array, dtype=np.uint8)
image = cv2.imdecode(array, cv2.IMREAD_COLOR)

# image = np.zeros((200,200,3), dtype = np.uint8)

cv2.imshow("image", image)
cv2.waitKey()
