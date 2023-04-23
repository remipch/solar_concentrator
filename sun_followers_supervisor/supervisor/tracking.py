import cv2
import numpy as np
import math
from motors_direction import MotorsDirection
from user_interface import *

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


previous_img = None
last_img = None

best_direction = MotorsDirection.LEFT

previous_spot_center = None

# print and flush
def prn(text):
    print(text,flush=True)

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
    showDebugImage("diff",diff_norm_img)

    removed_hot_blob_img = cv2.threshold(diff_norm_img, removed_hot_blob_max_level, 255, cv2.THRESH_TOZERO_INV)[1]
    new_hot_blob_img = cv2.threshold(diff_norm_img, new_hot_blob_min_level, 255, cv2.THRESH_TOZERO)[1]
    showDebugImage("blobs",removed_hot_blob_img+new_hot_blob_img)

    removed_blob_x,removed_blob_y = computeBlobCentroid(removed_hot_blob_img)
    new_blob_x,new_blob_y = computeBlobCentroid(new_hot_blob_img)

    spot_center_x = (removed_blob_x+new_blob_x)/2
    spot_center_y = (removed_blob_y+new_blob_y)/2
    prn(f"spot_center: {spot_center_x},{spot_center_y}")

    drawPoint(removed_blob_x,removed_blob_y,cyan)
    drawPoint(new_blob_x,new_blob_y,red)
    drawPoint(spot_center_x,spot_center_y,yellow)
    #drawCross(target_pos[0],target_pos[1],target_tol_px,green)

    return (spot_center_x,spot_center_y)

def compareAngles(a, b):
    diff = math.fmod(a - b, 2*math.pi)
    if diff > math.pi:
        diff -= 2*math.pi
    elif diff <= -math.pi:
        diff += 2*math.pi
    return diff

def updateMotorsDirectionHistory(direction,previous_center,current_center):
    angle_rad = math.atan2(current_center[1]-previous_center[1],current_center[0]-previous_center[0])
    prn(f"updateMotorsDirectionHistory: {direction} from {previous_center} to {current_center}({angle_rad} rad)")
    direction_angles[direction] = angle_rad

# return the best direction allowing to move from previous_center to wanted_center
def getBestMotorsDirection(previous_center,wanted_center):
    wanted_angle_rad = math.atan2(wanted_center[1]-previous_center[1],wanted_center[0]-previous_center[0])
    best_angle_diff = 2*math.pi
    best_direction = MotorsDirection.LEFT
    for direction, angle_rad in direction_angles.items():
        angle_diff = compareAngles(angle_rad,wanted_angle_rad)
        if angle_diff<best_angle_diff:
            best_angle_diff = angle_diff
            best_direction = direction
    prn(f"getBestMotorsDirection: {angle_rad} rad -> {direction}")
    return direction

# return the motors direction to move for the first step
def startTrackingOneStep(current_img,reset_tracking):
    global previous_img, previous_spot_center

    if reset_tracking:
        previous_spot_center = None

    previous_img = current_img
    return best_direction

# return the error between computed spot center and target pos in pixels
def finishTrackingOneStep(current_img,target_pos):
    global last_img, best_direction, previous_spot_center

    last_img = current_img

    showDebugImage("previous",previous_img)
    showDebugImage("last",last_img)

    spot_center = computeSpotCenter(previous_img,current_img)
    if previous_spot_center is not None:
        updateMotorsDirectionHistory(direction,previous_spot_center,spot_center)
        best_direction = getBestMotorsDirection(previous_spot_center,target_pos)
    previous_spot_center = spot_center
    target_error_px = math.dist(spot_center, target_pos)
    prn(f"target_error_px: {target_error_px}")
    return target_error_px


