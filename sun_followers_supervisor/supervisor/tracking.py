import cv2
import numpy as np
import math
from motors_direction import MotorsDirection
from user_interface import *
from panel_web_access import *

# threshold to detect start blobs and finish blobs from diff image
removed_hot_blob_max_level = 45
new_hot_blob_min_level = 220

# dictionary associating each direction to its delta in pixel
# initialized by predefined value
# update after each real move
direction_delta_px = {
    MotorsDirection.UP_RIGHT  : [10,-10],
    MotorsDirection.RIGHT     : [10,0],
    MotorsDirection.DOWN_RIGHT: [10,10],
    MotorsDirection.DOWN_LEFT : [-10,-10],
    MotorsDirection.LEFT      : [-10,0],
    MotorsDirection.UP_LEFT   : [-10,10],
}

target_pos_px = None

# tolerance arround target
TARGET_TOL_PX = 20

previous_img = None

current_direction = MotorsDirection.LEFT

previous_spot_center_px = None

def setTarget(target_pos):
    global target_pos_px
    print(f"NEW TARGET: {target_pos}",flush=True)
    target_pos_px = target_pos

def resetTarget():
    print(f"RESET TARGET",flush=True)
    target_pos_px = None

def drawTarget():
    if target_pos_px is not None:
        drawCross(target_pos_px[0],target_pos_px[1],TARGET_TOL_PX,green)


def computeBlobCentroid(blob_img):
    M = cv2.moments(blob_img)
    if M["m00"] == 0:
        raise Exception("Cannot compute blob centroid")
    x = int(M["m10"] / M["m00"])
    y = int(M["m01"] / M["m00"])
    return x,y

# return spot center in pixel and the move direction in pixel
def findSpot(previous_img,current_img):
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
    print(f"spot_center_px: {spot_center_x},{spot_center_y}",flush=True)

    drawPoint(removed_blob_x,removed_blob_y,cyan)
    drawPoint(new_blob_x,new_blob_y,red)
    drawPoint(spot_center_x,spot_center_y,yellow)
    drawTarget()

    spot_center_px = [spot_center_x,spot_center_y]
    move_direction_px = [new_blob_x-removed_blob_x, new_blob_y-removed_blob_y]

    return spot_center_px, move_direction_px

def updateMotorsDirectionHistory(direction, delta_px):
    print(f"updateMotorsDirectionHistory: {direction} delta_px: {delta_px}",flush=True)
    direction_delta_px[direction] = delta_px

# return the best direction allowing to move from previous_center_px to wanted_center
def getBestMotorsDirection(current_center_px, wanted_center_px):
    wanted_delta_px = [wanted_center_px[0]-current_center_px[0] , wanted_center_px[1]-current_center_px[1]]
    print(f"getBestMotorsDirection: wanted_delta_px: {wanted_delta_px}",flush=True)
    best_distance_px = 10000
    best_direction = None
    for direction, delta_px in direction_delta_px.items():
        center_px = [current_center_px[0]+delta_px[0] , current_center_px[1]+delta_px[1]]
        distance_px = math.dist(wanted_center_px, center_px)
        print(f"  test {direction}: center_px: {center_px} (distance_px: {distance_px})",flush=True)
        if distance_px<best_distance_px:
            best_distance_px = distance_px
            best_direction = direction
    print(f"  best_direction: {best_direction}",flush=True)
    return best_direction

def startTrackingOneStep(current_img, reset_tracking):
    global current_direction, previous_img, previous_spot_center_px
    print(f"startTrackingOneStep (reset_tracking: {reset_tracking}",flush=True)

    if reset_tracking:
        previous_spot_center_px = None

    if previous_spot_center_px is not None:
        current_direction = getBestMotorsDirection(previous_spot_center_px,target_pos_px)

    previous_img = current_img
    moveOneStep(current_direction)

# return True if target has been reached
def finishTrackingOneStep(current_img):
    global previous_spot_center_px

    showDebugImage("previous",previous_img)
    showDebugImage("last",current_img)

    current_center_px, move_direction_px = findSpot(previous_img,current_img)
    updateMotorsDirectionHistory(current_direction,move_direction_px)
    previous_spot_center_px = current_center_px
    target_error_px = math.dist(current_center_px, target_pos_px)
    print(f"target_error_px: {target_error_px}",flush=True)
    return (target_error_px < TARGET_TOL_PX)


