import cv2
import numpy as np

red = (0,0,204)
cyan = (204,204,0)
yellow = (0,255,255)
green = (0,204,0)
gray = (128,128,128)

# dictionary to map img name to image
debug_images = {}

# dictionary with last (left button) clic event data
last_left_clic_pos = None
last_left_clic_flags = None

# update opencv UI for 50 ms
# return 3 values :
# - the key pressed (or -1 if no key was pressed during this update period)
# - the last left clic pos (or None if no left clic from last call)
# - the last left clic flags (or None if no left clic from last call)
def updateUserInterface():
    global last_left_clic_pos,last_left_clic_flags

    key = cv2.waitKey(50)
    if key>=0:
        print(f"key pressed: {key}", flush=True)

    left_clic_pos = last_left_clic_pos
    last_left_clic_pos = None

    left_clic_flags = last_left_clic_flags
    last_left_clic_flags = None

    return key, left_clic_pos, left_clic_flags

def onMouse(event,x,y,flags,img_name):
    global last_left_clic_pos,last_left_clic_flags

    img = debug_images[img_name]

    if event == cv2.EVENT_LBUTTONDOWN:
        # store the event, it will be returned by next call to 'updateUserInterface'
        last_left_clic_pos = [x,y]
        last_left_clic_flags = flags
        #print(f"clic_pos: {last_left_clic_pos} ; clic_flags: {last_left_clic_flags}",flush=True)

def showDebugImage(img_name,img):
    #print(f"show debug image {img_name}",flush=True)
    debug_img = cv2.cvtColor(img, cv2.COLOR_GRAY2BGR)
    cv2.imshow(img_name, debug_img)
    cv2.setMouseCallback(img_name, onMouse, img_name)
    debug_images[img_name] = debug_img
    return debug_img

# draw a point in all debug images
def invPoint(x,y):
    for img_name, img in debug_images.items():
        img[int(y),int(x)] = 255-img[int(y),int(x)]
        cv2.imshow(img_name, img)

# draw a point in all debug images
def drawPoint(x,y,color):
    for img_name, img in debug_images.items():
        img[int(y),int(x)] = color
        cv2.imshow(img_name, img)

# draw a line in all debug images
def drawLine(x0,y0,x1,y1,color):
    for img_name, img in debug_images.items():
        cv2.line(img,(x0,y0),(x1,y1),color)
        cv2.imshow(img_name, img)

# draw a cross in all debug images
def drawCross(x,y,width,color):
    drawLine(x-width,y,x+width,y,color)
    drawLine(x,y-width,x,y+width,color)

