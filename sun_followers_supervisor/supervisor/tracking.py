import cv2
import math
from motors_direction import MotorsDirection
from user_interface import *
from panel_web_access import *

BLACK = (0,0,0)
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
REMOVED_HOT_BLOB_MAX_LEVEL = 64
NEW_HOT_BLOB_MIN_LEVEL = 192

# dictionary associating each direction to its delta in pixel
# initialized by predefined value
# update after each real move
direction_delta_px = {
    MotorsDirection.UP_LEFT      : [-10,-10],
    MotorsDirection.UP_RIGHT     : [10,-10],
    MotorsDirection.DOWN_RIGHT   : [10,10],
    MotorsDirection.DOWN_LEFT    : [-10,10],
}

target_pos_px = None

# Estimation of spot light size on the target area
# It's used to compute some limits to determine
# if various image detections are realistic or not
# Following value correspond approximatively to :
# - 800x600 camera resolution
# - 15x15 cm square spotlight on target area
# - 3 meters between camera and target area
SPOT_SIZE_PX = 40

# tolerance arround target
TARGET_TOL_PX = SPOT_SIZE_PX/4

# Limits used to check if a move is realistic (detect bad spot detections)
# (motors command are chosen so there is always an overlap between two successive spotlight after one step)
MIN_MOVE_PX = SPOT_SIZE_PX/4
MAX_MOVE_PX = SPOT_SIZE_PX
MIN_BLOB_AREA_PX = SPOT_SIZE_PX*SPOT_SIZE_PX/16
MAX_BLOB_AREA_PX = SPOT_SIZE_PX*SPOT_SIZE_PX

previous_img = None

current_direction = list(direction_delta_px.keys())[0]

previous_spot_center_px = None

# contains eventually 2 corners defining the region of interrest in which computation
# must be restricted (pixels outside of this ROI are ignored)
roi_corners_px = []
top_left_roi_corner_px = None
bottom_right_roi_corner_px = None

class TrackingException(Exception):
    "Cannot compute tracking"
    pass

def setTarget(target_pos):
    global target_pos_px
    print(f"NEW TARGET: {target_pos}",flush=True)
    if target_pos is not None and top_left_roi_corner_px is not None and bottom_right_roi_corner_px is not None:
        if target_pos[0]<top_left_roi_corner_px[0] or target_pos[1]<top_left_roi_corner_px[1] or target_pos[0]>bottom_right_roi_corner_px[0] or target_pos[1]>bottom_right_roi_corner_px[1]:
            print(f"TARGET is outside ROI -> reset target",flush=True)
            target_pos = None
    target_pos_px = target_pos
    closeDebugImage("previous")
    closeDebugImage("diff")
    closeDebugImage("blobs")

def isTargetSet():
    if target_pos_px is not None:
        return True
    else:
        return False

def setRoiCorner(roi_corner_px):
    global roi_corners_px,top_left_roi_corner_px,bottom_right_roi_corner_px
    print(f"NEW ROI CORNER: {roi_corner_px}",flush=True)
    if len(roi_corners_px)==0:
        roi_corners_px = [roi_corner_px]
    else:
        roi_corners_px.append(roi_corner_px)
        if len(roi_corners_px)>2:
            roi_corners_px.pop(0)

        top_left_roi_corner_px = (min(roi_corners_px[0][0],roi_corners_px[1][0]), min(roi_corners_px[0][1],roi_corners_px[1][1]))
        bottom_right_roi_corner_px = (max(roi_corners_px[0][0],roi_corners_px[1][0]), max(roi_corners_px[0][1],roi_corners_px[1][1]))
        print(f"top_left_roi_corner_px: {top_left_roi_corner_px}",flush=True)
        print(f"bottom_right_roi_corner_px: {bottom_right_roi_corner_px}",flush=True)

def resetTarget():
    global direction_delta_px
    direction_delta_px = {
        MotorsDirection.UP_LEFT      : [-10,-10],
        MotorsDirection.UP_RIGHT     : [10,-10],
        MotorsDirection.DOWN_RIGHT   : [10,10],
        MotorsDirection.DOWN_LEFT    : [-10,10],
    }
    setTarget(None)

def drawTarget():
    if target_pos_px is not None:
        drawCross(target_pos_px[0],target_pos_px[1],TARGET_TOL_PX,TARGET_COLOR)

def drawRoi():
    if top_left_roi_corner_px is not None and bottom_right_roi_corner_px is not None:
        drawRectangle(top_left_roi_corner_px,bottom_right_roi_corner_px,TARGET_COLOR)


def computeBlobCentroid(blob_img):
    M = cv2.moments(blob_img)
    if M["m00"] == 0:
        raise TrackingException("Cannot compute blob centroid")
    x = M["m10"] / M["m00"]
    y = M["m01"] / M["m00"]
    return x,y

# erase everything outside of ROI
# keeping the same size for all images make things simpler (coordinates are valid on all images)
def keepOnlyRoi(img):
    if top_left_roi_corner_px is None or bottom_right_roi_corner_px is None:
        return img

    result = img
    height, width = img.shape
    cv2.rectangle(result,(0,0),(top_left_roi_corner_px[0],height),BLACK,cv2.FILLED)
    cv2.rectangle(result,(bottom_right_roi_corner_px[0],0),(width,height),BLACK,cv2.FILLED)
    cv2.rectangle(result,(0,0),(width,top_left_roi_corner_px[1]),BLACK,cv2.FILLED)
    cv2.rectangle(result,(0,bottom_right_roi_corner_px[1]),(width,height),BLACK,cv2.FILLED)
    return result

# return spot center in pixel and the move direction in pixel
def findSpot(previous_img,current_img):
    previous_img = keepOnlyRoi(previous_img)
    current_img = keepOnlyRoi(current_img)

    diff_img = (128+cv2.divide(current_img, 2))-cv2.divide(previous_img, 2)
    diff_norm_img = cv2.normalize(diff_img, None, alpha=0, beta=255, norm_type=cv2.NORM_MINMAX)
    showDebugImage("diff",diff_norm_img,800,620)

    removed_hot_blob_img = cv2.threshold(diff_norm_img, REMOVED_HOT_BLOB_MAX_LEVEL, 255, cv2.THRESH_TOZERO_INV)[1]
    new_hot_blob_img = cv2.threshold(diff_norm_img, NEW_HOT_BLOB_MIN_LEVEL, 255, cv2.THRESH_TOZERO)[1]
    showDebugImage("blobs",removed_hot_blob_img+new_hot_blob_img,1200,620)

    checkRealisticBlob(removed_hot_blob_img)
    checkRealisticBlob(new_hot_blob_img)

    removed_blob_x,removed_blob_y = computeBlobCentroid(removed_hot_blob_img)
    new_blob_x,new_blob_y = computeBlobCentroid(new_hot_blob_img)

    spot_center_x = (removed_blob_x+new_blob_x)/2
    spot_center_y = (removed_blob_y+new_blob_y)/2

    drawTarget()
    drawRoi()
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
        print(f"  test {direction}: delta_px: [{delta_px[0]:.1f},{delta_px[1]:.1f}] center_px: [{center_px[0]:.1f},{center_px[1]:.1f}] (distance_px: {distance_px:.1f})",flush=True)
        drawPoint(center_px[0],center_px[1],POSSIBLE_DIRECTION_COLOR)
        if distance_px<best_distance_px:
            best_distance_px = distance_px
            best_direction = direction
            best_direction_point = center_px
    print(f"  best_direction: {best_direction}",flush=True)
    drawPoint(best_direction_point[0],best_direction_point[1],CHOSEN_DIRECTION_COLOR)
    return best_direction

# raise an exception if the blob does not seem realistic
def checkRealisticBlob(blob_img):
    blob_area_px = np.count_nonzero((blob_img > 0))
    print(f"blob_area_px: {blob_area_px}",flush=True)
    if (blob_area_px < MIN_BLOB_AREA_PX):
        raise TrackingException(f"Blob is too small (blob_area_px < {MIN_BLOB_AREA_PX})")
    if (blob_area_px > MAX_BLOB_AREA_PX):
        raise TrackingException(f"Blob is too large (blob_area_px > {MAX_BLOB_AREA_PX})")

# raise an exception if the move does not seem realistic
def checkRealisticMove(spot_center_px, move_direction_px):
    # WTF there is math.dist but no math.norm...
    move_direction_norm_px = math.dist([0,0], move_direction_px)
    print(f"  move_direction_norm_px: {move_direction_norm_px}",flush=True)
    if move_direction_norm_px<MIN_MOVE_PX:
        raise TrackingException(f"Move direction is too small (move_direction_norm_px < {MIN_MOVE_PX})")
    if move_direction_norm_px>MAX_MOVE_PX:
        raise TrackingException(f"Move direction is too large (move_direction_norm_px > {MAX_MOVE_PX})")

    if previous_spot_center_px is not None:
        spot_move_px = math.dist(previous_spot_center_px,spot_center_px)
        print(f"  spot_move_px: {spot_move_px}",flush=True)
        if spot_move_px > MAX_MOVE_PX:
            raise TrackingException(f"Spot move is too large (spot_move_px > {MAX_MOVE_PX})")

def startTrackingOneStep(current_img, reset_tracking):
    global previous_img, previous_spot_center_px
    print(f"startTrackingOneStep (reset_tracking: {reset_tracking})",flush=True)

    if reset_tracking:
        previous_spot_center_px = None

    previous_img = current_img
    moveOneStep(current_direction)

# return True if target has been reached
def finishTrackingOneStep(current_img):
    global current_direction, previous_spot_center_px

    showDebugImage("previous",previous_img,400,0)
    showDebugImage("current",current_img)

    try:
        current_center_px, move_direction_px = findSpot(previous_img,current_img)
        print(f"finishTrackingOneStep:",flush=True)
        print(f"  previous_spot_center_px: {previous_spot_center_px}",flush=True)
        print(f"  current_center_px: {current_center_px}",flush=True)
        print(f"  move_direction_px: {move_direction_px} ",flush=True)
        checkRealisticMove(current_center_px, move_direction_px)
    except TrackingException as tracking_exception:
        print(tracking_exception,flush=True)
        print(" -> SKIP STEP",flush=True)
        return False

    updateMotorsDirectionHistory(current_direction, move_direction_px)
    # calling getBestMotorsDirection here instead of in 'startTrackingOneStep' allow to draw debug info on the same image
    current_direction = getBestMotorsDirection(current_center_px,target_pos_px)
    previous_spot_center_px = current_center_px
    target_error_px = math.dist(current_center_px, target_pos_px)
    print(f"target_error_px: {target_error_px}",flush=True)
    return (target_error_px < TARGET_TOL_PX)


