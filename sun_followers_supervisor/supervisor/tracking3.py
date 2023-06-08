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
DARK_CYAN = (128,102,0)

TRACKING_ERROR = RED
AREA_COLOR = ORANGE
BEFORE_SUN_MOVE = DARK_GREEN
AFTER_SUN_MOVE = GREEN
BEFORE_MOTORS_MOVE = DARK_CYAN
AFTER_MOTORS_MOVE = CYAN

LIGHTED_PIXEL_MIN_OFFSET = 70

MAX_TRACKING_STEPS_COUNT = 20

MIN_LIGHTED_PIXELS_COUNT = 10

MIN_SUN_MOVE_PX = 2

MAX_DISTANCE_FROM_CENTER_AXIS_PX = 5

MIN_DISTANCE_FROM_BORDER_PX = 5

MIN_SPOT_OVERRUN_PX = 3

PANELS_COUNT = 1
current_panel_index = 0

tracking_steps_count = 0

# contains eventually 2 corners defining the region of interrest in which computation
# must be restricted (pixels outside of this ROI are ignored)
# TODO use rectangle class
area_corners_px = []
area_left_px = None
area_top_px = None
area_right_px = None
area_bottom_px = None

sun_move_x = 0
sun_move_y = 0
motors_direction = None
spot_light_rectangle_before_motors_move = None
before_motors_move_img = None

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

    def __str__(self):
        return f"(x0:{self.x0}, y0:{self.y0}, x1:{self.x1}, y1:{self.y1})"

class Rectangle:
    def __init__(self, left_px, top_px, right_px, bottom_px):
        self.left_px = left_px
        self.top_px = top_px
        self.right_px = right_px
        self.bottom_px = bottom_px

    # draw the segment in debug images
    def draw(self,color):
        drawRectangle(self.left_px, self.top_px, self.right_px, self.bottom_px, color)

    def getCenter(self):
        return (self.left_px + self.right_px) / 2 , (self.top_px + self.bottom_px) / 2

    def __str__(self):
        return f"(left:{self.left_px}, top:{self.top_px}, right:{self.right_px}, bottom_:{self.bottom_px})"

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

def getArea():
    return area_left_px, area_top_px, area_right_px, area_bottom_px

def getSpotLightRectangleInArea(img):
    spot_light_left_px = None
    for x in range(area_left_px, area_right_px+1):
        segment = getVerticalAreaSegment(x)
        if countLightedPixelsInSegment(img, segment)>=MIN_LIGHTED_PIXELS_COUNT:
            spot_light_left_px = x
            break
    spot_light_right_px = None
    for x in range(area_right_px+1,area_left_px,-1):
        segment = getVerticalAreaSegment(x)
        if countLightedPixelsInSegment(img, segment)>=MIN_LIGHTED_PIXELS_COUNT:
            spot_light_right_px = x
            break
    spot_light_top_px = None
    for y in range(area_top_px,area_bottom_px+1):
        segment = getHorizontalAreaSegment(y)
        if countLightedPixelsInSegment(img, segment)>=MIN_LIGHTED_PIXELS_COUNT:
            spot_light_top_px = y
            break
    spot_light_bottom_px = None
    for y in range(area_bottom_px+1,area_top_px,-1):
        segment = getHorizontalAreaSegment(y)
        if countLightedPixelsInSegment(img, segment)>=MIN_LIGHTED_PIXELS_COUNT:
            spot_light_bottom_px = y
            break

    if spot_light_right_px is None or spot_light_bottom_px is None:
        return None

    return Rectangle(spot_light_left_px,spot_light_top_px,spot_light_right_px,spot_light_bottom_px)

def drawArea():
    if isAreaSet():
        drawRectangle(area_left_px,area_top_px,area_right_px,area_bottom_px,AREA_COLOR)

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
    # prevent false-positive if image is saturated :
    min_level = borders_min_level + LIGHTED_PIXEL_MIN_OFFSET
    return np.count_nonzero(segment_img >= min_level)

def getBorderSegment(border):
    if border==Border.LEFT:
        return Segment(area_left_px,area_top_px,area_left_px,area_bottom_px)
    elif border==Border.TOP:
        return Segment(area_left_px,area_top_px,area_right_px,area_top_px)
    elif border==Border.RIGHT:
        return Segment(area_right_px,area_top_px,area_right_px,area_bottom_px)
    elif border==Border.BOTTOM:
        return Segment(area_left_px,area_bottom_px,area_right_px,area_bottom_px)

def getVerticalAreaSegment(x):
    return Segment(int(x),area_top_px,int(x),area_bottom_px)

def getHorizontalAreaSegment(y):
    return Segment(area_left_px,int(y),area_right_px,int(y))

# start tracking if sun borders are lighted
# return True if tracking has started
def startTracking(before_sun_move_img, current_img):
    print(f"startTracking",flush=True)

    showDebugImage("before_move",before_sun_move_img,400,0)
    showDebugImage("current",current_img)
    drawArea()

    spot_light_rectangle_before_sun_move = getSpotLightRectangleInArea(before_sun_move_img)
    spot_light_rectangle = getSpotLightRectangleInArea(current_img)
    print(f"    spot_light_rectangle_before_sun_move: {spot_light_rectangle_before_sun_move}",flush=True)
    print(f"    spot_light_rectangle: {spot_light_rectangle}",flush=True)

    # For debug only :
    borders_min_level = getBordersMinLevel(current_img)
    print(f"    borders_min_level: {borders_min_level}",flush=True)

    if spot_light_rectangle_before_sun_move is None or spot_light_rectangle is None:
        print(f"    NO SPOT DETECTED",flush=True)
        return False

    spot_light_rectangle_before_sun_move.draw(BEFORE_SUN_MOVE)
    spot_light_rectangle.draw(AFTER_SUN_MOVE)

    spot_center_x_before_sun_move,spot_center_y_before_sun_move = spot_light_rectangle_before_sun_move.getCenter()
    spot_center_x,spot_center_y = spot_light_rectangle.getCenter()
    print(f"    spot_center: {spot_center_x}, {spot_center_y}",flush=True)

    global sun_move_x,sun_move_y
    sun_move_x = spot_center_x - spot_center_x_before_sun_move
    sun_move_y = spot_center_y - spot_center_y_before_sun_move
    print(f"    sun_move: {sun_move_x}, {sun_move_y}",flush=True)

    if math.sqrt(sun_move_x**2 + sun_move_y**2) < MIN_SUN_MOVE_PX:
        print(f"    NO SUN MOVE DETECTED",flush=True)
        return False

    borders_to_move_away = getBordersToMoveAway(spot_light_rectangle)
    if len(borders_to_move_away)==0:
        print(f"    NO BORDERS TO MOVE AWAY",flush=True)
        return False

    # Reset steps count
    global tracking_steps_count
    tracking_steps_count = 1

    # Start moving in best opposite direction
    global motors_direction
    motors_direction = getBestMotorsDirection(borders_to_move_away)
    moveOneStep(motors_direction)

    global spot_light_rectangle_before_motors_move,before_motors_move_img
    spot_light_rectangle_before_motors_move = spot_light_rectangle
    before_motors_move_img = current_img

    return True

# return True if tracking is finished
def updateTracking(current_img):
    global motors_direction

    print(f"updateTracking",flush=True)

    spot_light_rectangle = getSpotLightRectangleInArea(current_img)
    print(f"    spot_light_rectangle: {spot_light_rectangle}",flush=True)

    global spot_light_rectangle_before_motors_move,before_motors_move_img
    showDebugImage("before_move",before_motors_move_img)
    showDebugImage("current",current_img)
    drawArea()
    spot_light_rectangle_before_motors_move.draw(BEFORE_MOTORS_MOVE)
    spot_light_rectangle.draw(AFTER_MOTORS_MOVE)

    borders_to_move_away = getBordersToMoveAway(spot_light_rectangle)
    if len(borders_to_move_away)==0:
        print("    TRACKING FINISHED",flush=True)
        return True

    global tracking_steps_count
    print(f"    tracking_steps_count: {tracking_steps_count}",flush=True)
    if tracking_steps_count>MAX_TRACKING_STEPS_COUNT:
        raise Exception(f"Problem : cannot reach opposite borders after {tracking_steps_count} tracking steps")
    tracking_steps_count = tracking_steps_count + 1

    # Only for multi-panel : if the current moving panel has made an overrun -> use next panel
    # The (theoretical) problem with multi-panel is that a panel spot light
    # might be hidden by other panels spot lights. So moving current panel does not
    # contribute to decrease spot_error.
    # By testing if the current panel spot light makes an overrun in the motors_direction,
    # we ensure the current panel had an effect and we can move the next panel.
    # Repeating this until acceptable tolerance is reached seems good.
    # (NOT YET TESTED IN REAL LIFE but at least it works for one panel)
    spot_overrun_left = spot_light_rectangle_before_motors_move.left_px - spot_light_rectangle.left_px
    spot_overrun_top = spot_light_rectangle_before_motors_move.top_px - spot_light_rectangle.top_px
    spot_overrun_right = spot_light_rectangle.right_px - spot_light_rectangle_before_motors_move.right_px
    spot_overrun_bottom = spot_light_rectangle.bottom_px - spot_light_rectangle_before_motors_move.bottom_px
    spot_overrun_in_motors_direction = False

    if spot_overrun_left > MIN_SPOT_OVERRUN_PX and (
        motors_direction==MotorsDirection.UP_LEFT or motors_direction==MotorsDirection.LEFT or motors_direction==MotorsDirection.DOWN_LEFT):
        spot_overrun_in_motors_direction = True

    if spot_overrun_top > MIN_SPOT_OVERRUN_PX and (
        motors_direction==MotorsDirection.UP_LEFT or motors_direction==MotorsDirection.UP or motors_direction==MotorsDirection.UP_RIGHT):
        spot_overrun_in_motors_direction = True

    if spot_overrun_right > MIN_SPOT_OVERRUN_PX and (
        motors_direction==MotorsDirection.UP_RIGHT or motors_direction==MotorsDirection.RIGHT or motors_direction==MotorsDirection.DOWN_RIGHT):
        spot_overrun_in_motors_direction = True

    if spot_overrun_bottom > MIN_SPOT_OVERRUN_PX and (
        motors_direction==MotorsDirection.DOWN_LEFT or motors_direction==MotorsDirection.DOWN or motors_direction==MotorsDirection.DOWN_RIGHT):
        spot_overrun_in_motors_direction = True

    if PANELS_COUNT>1 and spot_overrun_in_motors_direction:
        spot_light_rectangle_before_motors_move = spot_light_rectangle
        before_motors_move_img = current_img

        current_panel_index = (current_panel_index+1) % PANELS_COUNT
        print(f"    current_panel_index: {current_panel_index}",flush=True)

    # Start moving in best opposite direction
    motors_direction = getBestMotorsDirection(borders_to_move_away)
    moveOneStep(motors_direction)# TODO for multi-panel : address current_panel_index

    return False

# Depending on sun_move global value, find borders to move away
def getBordersToMoveAway(spot_light_rectangle):
    distance_from_left_border = abs(spot_light_rectangle.left_px - area_left_px)
    distance_from_top_border = abs(spot_light_rectangle.top_px - area_top_px)
    distance_from_right_border = abs(spot_light_rectangle.right_px - area_right_px)
    distance_from_bottom_border = abs(spot_light_rectangle.bottom_px - area_bottom_px)
    print(f"    distance from borders: {distance_from_left_border}, {distance_from_top_border}, {distance_from_right_border}, {distance_from_bottom_border}",flush=True)

    spot_center_x,spot_center_y = spot_light_rectangle.getCenter()
    print(f"    spot_center: {spot_center_x}, {spot_center_y}",flush=True)

    area_center_x = (area_left_px+area_right_px)/2 # TODO use Rectangle.getCenter()
    area_center_y = (area_top_px+area_bottom_px)/2 # TODO use Rectangle.getCenter()

    spot_center_error_x = spot_center_x - area_center_x
    spot_center_error_y = spot_center_y - area_center_y
    print(f"    spot_center_error: {spot_center_error_x}, {spot_center_error_y}",flush=True)

    borders_to_move_away = []

    if sun_move_x<0:
        if distance_from_left_border < MIN_DISTANCE_FROM_BORDER_PX:
            borders_to_move_away.append(Border.LEFT)
            getBorderSegment(Border.LEFT).draw(TRACKING_ERROR)
        if -spot_center_error_x > MAX_DISTANCE_FROM_CENTER_AXIS_PX:
            borders_to_move_away.append(Border.LEFT)
            drawLine(spot_center_x,spot_center_y,area_center_x,spot_center_y,TRACKING_ERROR)

    if sun_move_x>0:
        if distance_from_right_border < MIN_DISTANCE_FROM_BORDER_PX:
            borders_to_move_away.append(Border.RIGHT)
            getBorderSegment(Border.RIGHT).draw(TRACKING_ERROR)
        if spot_center_error_x > MAX_DISTANCE_FROM_CENTER_AXIS_PX:
            borders_to_move_away.append(Border.RIGHT)
            drawLine(spot_center_x,spot_center_y,area_center_x,spot_center_y,TRACKING_ERROR)

    if sun_move_y<0:
        if distance_from_top_border < MIN_DISTANCE_FROM_BORDER_PX:
            borders_to_move_away.append(Border.TOP)
            getBorderSegment(Border.TOP).draw(TRACKING_ERROR)
        if -spot_center_error_y > MAX_DISTANCE_FROM_CENTER_AXIS_PX:
            borders_to_move_away.append(Border.TOP)
            drawLine(spot_center_x,spot_center_y,spot_center_x,area_center_y,TRACKING_ERROR)

    if sun_move_y>0:
        if distance_from_bottom_border < MIN_DISTANCE_FROM_BORDER_PX:
            borders_to_move_away.append(Border.BOTTOM)
            getBorderSegment(Border.BOTTOM).draw(TRACKING_ERROR)
        if spot_center_error_y > MAX_DISTANCE_FROM_CENTER_AXIS_PX:
            borders_to_move_away.append(Border.BOTTOM)
            drawLine(spot_center_x,spot_center_y,spot_center_x,area_center_y,TRACKING_ERROR)

    print(f"    borders_to_move_away: {borders_to_move_away}",flush=True)
    return borders_to_move_away

def getBestMotorsDirection(borders_to_move_away):
    if Border.LEFT in borders_to_move_away:
        if Border.TOP in borders_to_move_away:
            return MotorsDirection.DOWN_RIGHT
        elif Border.BOTTOM in borders_to_move_away:
            return MotorsDirection.UP_RIGHT
        else:
            return MotorsDirection.RIGHT
    elif Border.RIGHT in borders_to_move_away:
        if Border.TOP in borders_to_move_away:
            return MotorsDirection.DOWN_LEFT
        elif Border.BOTTOM in borders_to_move_away:
            return MotorsDirection.UP_LEFT
        else:
            return MotorsDirection.LEFT
    elif Border.TOP in borders_to_move_away:
        return MotorsDirection.DOWN
    elif Border.BOTTOM in borders_to_move_away:
        return MotorsDirection.UP
    else:
        raise Exception(f"Incorrect borders_to_move_away: {borders_to_move_away}")


