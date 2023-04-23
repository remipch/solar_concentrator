import cv2
import numpy as np
import math
from panel_web_access import *
from user_interface import *

# tolerance arround target
target_tol_px = 20

# defined by Ctrl+Clic on image
target_pos = None

# threshold to detect start blobs and finish blobs from diff image
removed_hot_blob_max_level = 45
new_hot_blob_min_level = 220

# dictionary associating direction to the most probable move angle obtained
# initialized by predefined value
# update after each real move
direction_angles = {}
angle = math.pi/3
for direction in MotorsDirection:
    direction_angles[direction] = angle
    angle = angle+math.pi/3

tracking_enabled = False

# replace all fixed-time wait by infinite wait for debug purpose
# press a key to unlock
wait_at_each_step = True

# print and flush
def prn(text):
    print(text,flush=True)

def setTargetAndStartTracking(target_x,target_y):
    global tracking_enabled,target_pos
    target_pos = (target_x,target_y)
    tracking_enabled = True
    draw_target()

def computeBlobCentroid(blob_img):
    M = cv2.moments(blob_img)
    if M["m00"] == 0:
        raise Exception("Cannot compute blob centroid")
    x = int(M["m10"] / M["m00"])
    y = int(M["m01"] / M["m00"])
    return x,y

def computeSpotCenter(previous_img,current_img):
    diff_img = (128+cv2.divide(current_img, 2))-cv2.divide(previous_img, 2)
    diff_norm_img = cv2.normalize(diff_img, None, alpha=0, beta=255, norm_type=cv2.NORM_MINMAX)
    showDebugImage("diff_norm",diff_norm_img)

    removed_hot_blob_img = cv2.threshold(diff_norm_img, removed_hot_blob_max_level, 255, cv2.THRESH_TOZERO_INV)[1]
    new_hot_blob_img = cv2.threshold(diff_norm_img, new_hot_blob_min_level, 255, cv2.THRESH_TOZERO)[1]
    showDebugImage("blobs",removed_hot_blob_img+new_hot_blob_img)

    removed_blob_x,removed_blob_y = computeBlobCentroid(removed_hot_blob_img)
    new_blob_x,new_blob_y = computeBlobCentroid(new_hot_blob_img)

    spot_center_x = (removed_blob_x+new_blob_x)/2
    spot_center_y = (removed_blob_y+new_blob_y)/2
    prn(f"spot_center: {spot_center_x},{spot_center_y}")

    draw_point(removed_blob_x,removed_blob_y,cyan)
    draw_point(new_blob_x,new_blob_y,red)
    draw_point(spot_center_x,spot_center_y,yellow)

    return (spot_center_x,spot_center_y)

def compareAngles(a, b):
    diff = math.fmod(a - b, 2*math.pi)
    if diff > math.pi:
        diff -= 2*math.pi
    elif diff <= -math.pi:
        diff += 2*math.pi
    return diff

def updateMotorsDirectionHistory(direction,previous_center,spot_center):
    angle_rad = math.atan2(spot_center[1]-previous_center[1],spot_center[0]-previous_center[0])
    prn(f"updateMotorsDirectionHistory: {direction} -> {angle_rad} rad")
    direction_angles[direction] = angle_rad

# return the best direction allowing to move from previous_center to spot_center
def getBestMotorsDirection(previous_center,spot_center):
    wanted_angle_rad = math.atan2(spot_center[1]-previous_center[1],spot_center[0]-previous_center[0])
    best_angle_diff = 2*math.pi
    best_direction = MotorsDirection.LEFT
    for direction, angle_rad in direction_angles.items():
        angle_diff = compareAngles(angle_rad,wanted_angle_rad)
        if angle_diff<best_angle_diff:
            best_angle_diff = angle_diff
            best_direction = direction
    prn(f"getBestMotorsDirection: {angle_rad} rad -> {direction}")
    return direction

def reachTarget():
    print("reachTarget")
    direction = MotorsDirection.LEFT # todo : initialize to most used direction (most probable)

    previous_img = cameraCapture()
    showDebugImage("previous",previous_img)
    draw_target()

    previous_spot_center = None
    target_error_px = 2*target_tol_px # force to run at least one iteration

    while target_error_px>target_tol_px:
        moveOneStep(direction)
        if wait_at_each_step:
            if updateUserInterface(0):
                return
        else:
            for i in range(5):
                if updateUserInterface(1000):
                    return
                get_motors_status() # todo :check 'locked'

        current_img = cameraCapture()
        showDebugImage("current",current_img)

        spot_center = computeSpotCenter(previous_img,current_img)
        if previous_spot_center is not None:
            updateMotorsDirectionHistory(direction,previous_spot_center,spot_center)
            direction = getBestMotorsDirection(previous_spot_center,spot_center)
        previous_img = current_img
        previous_spot_center = spot_center

        target_error_px = math.dist(spot_center , target_pos)
        prn(f"target_error_px: {target_error_px}")


#while True:
    #current_img = cameraCapture()
    #showDebugImage("current",current_img)
    #draw_target()

    #if wait_at_each_step:
        #updateUserInterface(0)
    #else:
        #updateUserInterface(10000)

    #if tracking_enabled:
        #reachTarget()
    #else:
        #previous_img = current_img
        #showDebugImage("previous",previous_img)



