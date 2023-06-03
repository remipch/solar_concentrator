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

SUN_BORDER_COLOR =                  ORANGE
LIGHTED_SUN_BORDER_COLOR =          RED
OPPOSITE_BORDER_COLOR =             BLUE
LIGHTED_OPPOSITE_BORDER_COLOR  =    CYAN

LIGHTED_PIXEL_MIN_OFFSET = 30
MAX_TRACKING_STEPS_COUNT = 5  # error if more step to reach opposite borders

MIN_LIGHTED_PIXELS_COUNT = 10


tracking_steps_count = 0

# contains eventually 2 corners defining the region of interrest in which computation
# must be restricted (pixels outside of this ROI are ignored)
area_corners_px = []
area_left_px = None
area_top_px = None
area_right_px = None
area_bottom_px = None

is_morning=None

motors_direction = None

class Border(str, Enum):
    LEFT   = "LEFT"
    TOP    = "TOP"
    RIGHT  = "RIGHT"
    BOTTOM = "BOTTOM"

ALL_BORDERS = [Border.LEFT,Border.TOP,Border.RIGHT,Border.BOTTOM]

class Segment:
    def __init__(self, x0, y0, x1, y1):
        self.x0 = x0
        self.y0 = y0
        self.x1 = x1
        self.y1 = y1

    # draw the segment in debug images
    def draw(self,color):
        drawLine(self.x0, self.y0, self.x1, self.y1, color)

    def getSubImage(self,img):
        return img[self.y0:self.y1+1 , self.x0:self.x1+1]

def setMorning(is_morning_):
    global is_morning
    is_morning = is_morning_
    print(f"is_morning: {is_morning}",flush=True)

def setAreaCorner(corner_px):
    global area_corners_px,area_top_px,area_right_px,area_bottom_px,area_left_px
    print(f"NEW AREA CORNER: {corner_px}",flush=True)
    if len(area_corners_px)==0:
        area_corners_px = [corner_px]
    else:
        area_corners_px.append(corner_px)
        if len(area_corners_px)>2:
            area_corners_px.pop(0)

        area_left_px = min(area_corners_px[0][0],area_corners_px[1][0])
        area_top_px = min(area_corners_px[0][1],area_corners_px[1][1])
        area_right_px = max(area_corners_px[0][0],area_corners_px[1][0])
        area_bottom_px = max(area_corners_px[0][1],area_corners_px[1][1])
        print(f"area_left_px: {area_left_px}",flush=True)
        print(f"area_top_px: {area_top_px}",flush=True)
        print(f"area_right_px: {area_right_px}",flush=True)
        print(f"area_bottom_px: {area_bottom_px}",flush=True)

def isAreaSet():
    return (len(area_corners_px)==2)

def resetArea():
    global area_corners_px,area_left_px,area_top_px,area_right_px,area_bottom_px
    print(f"reset area",flush=True)
    area_corners_px = []
    area_left_px = None
    area_top_px = None
    area_right_px = None
    area_bottom_px = None

def drawArea(img):
    if isAreaSet():
        for border in getSunBorders():
            segment = getBorderSegment(border)
            if countLightedPixelsInSegment(img,segment)>=MIN_LIGHTED_PIXELS_COUNT:
                segment.draw(LIGHTED_SUN_BORDER_COLOR)
            else:
                segment.draw(SUN_BORDER_COLOR)
        for border in getOppositeBorders():
            segment = getBorderSegment(border)
            if countLightedPixelsInSegment(img,segment)>=MIN_LIGHTED_PIXELS_COUNT:
                segment.draw(LIGHTED_OPPOSITE_BORDER_COLOR)
            else:
                segment.draw(OPPOSITE_BORDER_COLOR)

def getSunBorders():
    if is_morning:
        return [Border.LEFT,Border.BOTTOM]
    else:
        return [Border.LEFT,Border.TOP]

def getOppositeBorders():
    if is_morning:
        return [Border.RIGHT,Border.TOP]
    else:
        return [Border.RIGHT,Border.BOTTOM]

def getBordersMinLevel(img):
    borders_min_level = 255
    for border in ALL_BORDERS:
        segment = getBorderSegment(border)
        segment_img = segment.getSubImage(img)
        borders_min_level = min(borders_min_level, segment_img.min())
    return borders_min_level

def countLightedPixelsInSegment(img,segment):
    borders_min_level = getBordersMinLevel(img)
    segment_img = segment.getSubImage(img)
    return np.count_nonzero(segment_img >= borders_min_level + LIGHTED_PIXEL_MIN_OFFSET)

def getBorderSegment(border):
    if border==Border.LEFT:
        return Segment(area_left_px,area_top_px,area_left_px,area_bottom_px)
    elif border==Border.TOP:
        return Segment(area_left_px,area_top_px,area_right_px,area_top_px)
    elif border==Border.RIGHT:
        return Segment(area_right_px,area_top_px,area_right_px,area_bottom_px)
    elif border==Border.BOTTOM:
        return Segment(area_left_px,area_bottom_px,area_right_px,area_bottom_px)

# return an array containing all the lighted borders from the given 'borders_to_test'
def getLightedBorders(img, borders_to_test):
    lighted_borders = []
    for border in borders_to_test:
        segment = getBorderSegment(border)
        if countLightedPixelsInSegment(img,segment) > MIN_LIGHTED_PIXELS_COUNT:
            lighted_borders.append(border)
    return lighted_borders

# start tracking if sun borders are lighted
# return True if tracking has started
def startTracking(img):
    # Search the most lighted sun border
    lighted_sun_borders = getLightedBorders(img,getSunBorders())
    if len(lighted_sun_borders)==0:
        return False

    print(f"lighted_sun_borders: {lighted_sun_borders}",flush=True)

    # Reset steps count
    global tracking_steps_count
    tracking_steps_count = 1

    # Start moving in best opposite direction
    global motors_direction
    motors_direction = getBestMotorsDirection(lighted_sun_borders)
    moveOneStep(motors_direction)

    return True

# return True if tracking is finished
def updateTracking(img):
    global tracking_steps_count
    print(f"tracking_steps_count: {tracking_steps_count}",flush=True)

    lighted_opposite_borders = getLightedBorders(img,getOppositeBorders())
    print(f"lighted_opposite_borders: {lighted_opposite_borders}",flush=True)
    if len(lighted_opposite_borders)>0:
        return True

    if tracking_steps_count>MAX_TRACKING_STEPS_COUNT:
        raise Exception(f"Problem : cannot reach opposite borders after {tracking_steps_count} tracking steps")
    tracking_steps_count = tracking_steps_count + 1

    moveOneStep(motors_direction)

    return False

def getBestMotorsDirection(lighted_sun_borders):
    if Border.LEFT in lighted_sun_borders:
        if Border.TOP in lighted_sun_borders:
            return MotorsDirection.BOTTOM_RIGHT
        elif Border.BOTTOM in lighted_sun_borders:
            return MotorsDirection.UP_RIGHT
        else:
            return MotorsDirection.RIGHT
    elif Border.TOP in lighted_sun_borders:
        return MotorsDirection.DOWN
    elif Border.BOTTOM in lighted_sun_borders:
        return MotorsDirection.UP
    else:
        raise Exception(f"Incorrect lighted_sun_borders: {lighted_sun_borders}")


