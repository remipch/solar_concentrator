import cv2
import math
from motors_direction import MotorsDirection
from user_interface import *
from panel_web_access import *

BLUE = (255,102,0)
ORANGE = (0,153,255)
GREEN = (0,204,0)
DARK_GREEN = (0,102,0)
RED = (40,40,213)
CYAN = (255,204,0)

TARGET_COLOR =              GREEN
PREVIOUS_CENTER_COLOR =     RED
CURRENT_CENTER_COLOR =      ORANGE
#REMOVED_BLOB_COLOR =        CYAN
#NEW_BLOB_COLOR =            BLUE
POSSIBLE_DIRECTION_COLOR =  BLUE
CHOSEN_DIRECTION_COLOR =    CYAN

# threshold to detect start blobs and finish blobs from diff image
removed_hot_blob_max_level = 64
new_hot_blob_min_level = 192

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
TARGET_TOL_PX = 10

# to check if a move is realistic (detect bad spot detections)
MIN_MOVE_DIRECTION_PX = 3
MAX_MOVE_DIRECTION_PX = 20
MAX_MOVE_PX = 10

previous_img = None

current_direction = MotorsDirection.LEFT

previous_spot_center_px = None

def setTarget(target_pos):
    global target_pos_px
    print(f"NEW TARGET: {target_pos}",flush=True)
    target_pos_px = target_pos
    closeDebugImage("previous")
    closeDebugImage("diff")
    closeDebugImage("blobs")

def resetTarget():
    setTarget(None)

def drawTarget():
    if target_pos_px is not None:
        drawCross(target_pos_px[0],target_pos_px[1],TARGET_TOL_PX,TARGET_COLOR)


def computeBlobCentroid(blob_img):
    M = cv2.moments(blob_img)
    if M["m00"] == 0:
        raise Exception("Cannot compute blob centroid")
    x = M["m10"] / M["m00"]
    y = M["m01"] / M["m00"]
    return x,y

# return spot center in pixel and the move direction in pixel
def findSpot(previous_img,current_img):
    diff_img = (128+cv2.divide(current_img, 2))-cv2.divide(previous_img, 2)
    diff_norm_img = cv2.normalize(diff_img, None, alpha=0, beta=255, norm_type=cv2.NORM_MINMAX)
    showDebugImage("diff",diff_norm_img,800,400)

    removed_hot_blob_img = cv2.threshold(diff_norm_img, removed_hot_blob_max_level, 255, cv2.THRESH_TOZERO_INV)[1]
    new_hot_blob_img = cv2.threshold(diff_norm_img, new_hot_blob_min_level, 255, cv2.THRESH_TOZERO)[1]
    showDebugImage("blobs",removed_hot_blob_img+new_hot_blob_img,1200,400)

    removed_blob_x,removed_blob_y = computeBlobCentroid(removed_hot_blob_img)
    new_blob_x,new_blob_y = computeBlobCentroid(new_hot_blob_img)

    spot_center_x = (removed_blob_x+new_blob_x)/2
    spot_center_y = (removed_blob_y+new_blob_y)/2

    drawTarget()
    if previous_spot_center_px is not None:
        drawCross(previous_spot_center_px[0],previous_spot_center_px[1],2,PREVIOUS_CENTER_COLOR)
    #drawCross(spot_center_x,spot_center_y,2,CURRENT_CENTER_COLOR)
    #drawPoint(removed_blob_x,removed_blob_y,REMOVED_BLOB_COLOR)
    #drawPoint(new_blob_x,new_blob_y,NEW_BLOB_COLOR)
    drawLine(removed_blob_x,removed_blob_y,new_blob_x,new_blob_y,CURRENT_CENTER_COLOR)


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
    best_direction_point = None
    for direction, delta_px in direction_delta_px.items():
        center_px = [current_center_px[0]+delta_px[0] , current_center_px[1]+delta_px[1]]
        distance_px = math.dist(wanted_center_px, center_px)
        print(f"  test {direction}: center_px: {center_px} (distance_px: {distance_px})",flush=True)
        drawPoint(center_px[0],center_px[1],POSSIBLE_DIRECTION_COLOR)
        if distance_px<best_distance_px:
            best_distance_px = distance_px
            best_direction = direction
            best_direction_point = center_px
    print(f"  best_direction: {best_direction}",flush=True)
    drawPoint(best_direction_point[0],best_direction_point[1],CHOSEN_DIRECTION_COLOR)
    return best_direction

# return True if the move seems realistic
def isRealisticMove(spot_center_px, move_direction_px):
    # WTF there is math.dist but no math.norm...
    move_direction_norm_px = math.dist([0,0], move_direction_px)
    if move_direction_norm_px<MIN_MOVE_DIRECTION_PX:
        print(f"  move_direction_norm_px: {move_direction_norm_px} < {MIN_MOVE_DIRECTION_PX}",flush=True)
        return False
    if move_direction_norm_px>MAX_MOVE_DIRECTION_PX:
        print(f"  move_direction_norm_px: {move_direction_norm_px} > {MAX_MOVE_DIRECTION_PX}",flush=True)
        return False

    if previous_spot_center_px is not None:
        spot_move_px = math.dist(previous_spot_center_px,spot_center_px)
        if spot_move_px > MAX_MOVE_PX:
            print(f"  spot_move_px: {spot_move_px} > {MAX_MOVE_PX}",flush=True)
            return False

    return True

def startTrackingOneStep(current_img, reset_tracking):
    global previous_img, previous_spot_center_px
    print(f"startTrackingOneStep (reset_tracking: {reset_tracking}",flush=True)

    if reset_tracking:
        previous_spot_center_px = None

    previous_img = current_img
    moveOneStep(current_direction)

# return True if target has been reached
def finishTrackingOneStep(current_img):
    global current_direction, previous_spot_center_px

    showDebugImage("previous",previous_img,800,0)
    showDebugImage("current",current_img,1200,0)

    current_center_px, move_direction_px = findSpot(previous_img,current_img)
    print(f"finishTrackingOneStep:",flush=True)
    print(f"  previous_spot_center_px: {previous_spot_center_px}",flush=True)
    print(f"  current_center_px: {current_center_px}",flush=True)
    print(f"  move_direction_px: {move_direction_px} ",flush=True)

    if not isRealisticMove(current_center_px, move_direction_px):
        print(" -> SKIP STEP",flush=True)
        previous_spot_center_px = current_center_px
        return False

    updateMotorsDirectionHistory(current_direction, move_direction_px)
    # calling getBestMotorsDirection here instead of in 'startTrackingOneStep' allow to draw debug info on the same image
    current_direction = getBestMotorsDirection(current_center_px,target_pos_px)
    previous_spot_center_px = current_center_px
    target_error_px = math.dist(current_center_px, target_pos_px)
    print(f"target_error_px: {target_error_px}",flush=True)
    return (target_error_px < TARGET_TOL_PX)


