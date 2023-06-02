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

LIGHTED_PIXEL_MIN_LEVEL = 50
MIN_LIGHTED_PIXELS_COUNT = 10

# contains eventually 2 corners defining the region of interrest in which computation
# must be restricted (pixels outside of this ROI are ignored)
area_corners_px = []
area_left_px = None
area_top_px = None
area_right_px = None
area_bottom_px = None

is_morning=None

initial_lighted_pixels_count_in_opposite_borders = 0

current_direction = None

class Border(str, Enum):
    LEFT   = "LEFT"
    TOP    = "TOP"
    RIGHT  = "RIGHT"
    BOTTOM = "BOTTOM"

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

class TrackingException(Exception):
    "Cannot compute tracking"
    pass

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

def countLightedPixelsInSegment(img,segment):
    segment_img = segment.getSubImage(img)
    return np.count_nonzero(segment_img >= LIGHTED_PIXEL_MIN_LEVEL)

def getBorderSegment(border):
    if border==Border.LEFT:
        return Segment(area_left_px,area_top_px,area_left_px,area_bottom_px)
    elif border==Border.TOP:
        return Segment(area_left_px,area_top_px,area_right_px,area_top_px)
    elif border==Border.RIGHT:
        return Segment(area_right_px,area_top_px,area_right_px,area_bottom_px)
    elif border==Border.BOTTOM:
        return Segment(area_left_px,area_bottom_px,area_right_px,area_bottom_px)

# start tracking if sun borders are lighted
# return True if tracking has started
def startTracking(img):
    # Search the most lighted sun border
    max_lighted_pixels_count = 0
    most_lighted_sun_border = None
    for sun_border in getSunBorders():
        segment = getBorderSegment(sun_border)
        lighted_pixels_count = countLightedPixelsInSegment(img,segment)
        if lighted_pixels_count > max_lighted_pixels_count:
            max_lighted_pixels_count = lighted_pixels_count
            most_lighted_sun_border = sun_border
    if most_lighted_sun_border is None:
        return False


    print(f"most_lighted_sun_border: {most_lighted_sun_border} ; lighted_pixels_count: {lighted_pixels_count}",flush=True)

    # Store the lighted pixels count in opposite borders
    # it will be used later to determine when spot has reached the opposite borders
    global initial_lighted_pixels_count_in_opposite_borders
    initial_lighted_pixels_count_in_opposite_borders = countLightedPixelsInOppositeBorders(img)

    # Start moving in best opposite direction
    global current_direction
    current_direction = getBestMotorsDirection(most_lighted_sun_border)
    moveOneStep(current_direction)

    return True

# return True if tracking is finished
def updateTracking(img):
    global initial_lighted_pixels_count_in_opposite_borders
    lighted_pixels_count_in_opposite_borders = countLightedPixelsInOppositeBorders(img)
    print(f"lighted_pixels_count_in_opposite_borders: {lighted_pixels_count_in_opposite_borders}",flush=True)
    if lighted_pixels_count_in_opposite_borders > initial_lighted_pixels_count_in_opposite_borders + MIN_LIGHTED_PIXELS_COUNT:
        return True

    if lighted_pixels_count_in_opposite_borders < initial_lighted_pixels_count_in_opposite_borders:
        initial_lighted_pixels_count_in_opposite_borders = lighted_pixels_count_in_opposite_borders

    moveOneStep(current_direction)

    return False

def countLightedPixelsInOppositeBorders(img):
    lighted_pixels_count = 0
    for opposite_border in getOppositeBorders():
        segment = getBorderSegment(opposite_border)
        lighted_pixels_count += countLightedPixelsInSegment(img,segment)
    return lighted_pixels_count

def getBestMotorsDirection(most_lighted_sun_border):
    if most_lighted_sun_border == Border.LEFT:
        return MotorsDirection.RIGHT
    elif most_lighted_sun_border == Border.TOP:
        return MotorsDirection.DOWN
    elif most_lighted_sun_border == Border.BOTTOM:
        return MotorsDirection.UP


