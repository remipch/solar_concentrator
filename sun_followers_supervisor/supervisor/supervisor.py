# main supervisor script, just run it

import cv2
import numpy as np
import math
from panel_web_access import *
from user_interface import *
from state import *
from tracking3 import *
import traceback
import time

pause_after_each_step = True

CAPTURE_ONLY_AREA_IN_TRACKING = False
#CAPTURE_ONLY_AREA_IN_TRACKING = True

# user actions
ESCAPE_KEY = 27     # quit progam
SPACE_KEY = 32      # skip a waiting state
DELETE_KEY = 255    # reset target pos
P_KEY = 112         # toggle 'pause_after_each_step'
N0_KEY = 48         # 0 : stop move
N8_KEY = 56         # 8 : up step
N9_KEY = 57         # 9 : top-right step
N6_KEY = 54         # 6 : right step
N3_KEY = 51         # 3 : down-right step
N2_KEY = 50         # 2 : down step
N1_KEY = 49         # 1 : down-left step
N4_KEY = 52         # 4 : left step
N7_KEY = 55         # 7 : top-left step

def drawCurrentImage():
    showDebugImage("current",current_img,800,0,800,600)
    drawArea()

# initialisation
current_img = cameraCapture()
before_sun_move_img = None
drawCurrentImage()
setState(State.WAITING_AREA_DEFINITION)

# After a manual move
def waitUntilMotorsStop():
    while True:
        status = getMotorsStatus()
        if status==MotorsStatus.MOVING_ONE_STEP:
            continue
        elif status==MotorsStatus.LOCKED:
            return
        else:
            raise Exception(f"Incorrect motors status: {status}")
        time.sleep(1)

# application main loop
try:
    while True:
        key, clic_pos, clic_flags = updateUserInterface()
        state, state_duration_s = getState()

        # Treat common key press (meaningful for all states)

        # Exit application on escape
        if key==ESCAPE_KEY:
            exit(0)

        # Manual move don't change the state machine :
        # - move one step
        # - wait motors stop
        # - stay in the same state
        # (it allows to simulate sun move manually at any state)
        elif key==N8_KEY:
            moveOneStep(MotorsDirection.UP)
            waitUntilMotorsStop()
        elif key==N1_KEY:
            moveOneStep(MotorsDirection.DOWN_LEFT)
            waitUntilMotorsStop()
        elif key==N4_KEY:
            moveOneStep(MotorsDirection.LEFT)
            waitUntilMotorsStop()
        elif key==N7_KEY:
            moveOneStep(MotorsDirection.UP_LEFT)
            waitUntilMotorsStop()
        elif key==N2_KEY:
            moveOneStep(MotorsDirection.DOWN)
            waitUntilMotorsStop()
        elif key==N3_KEY:
            moveOneStep(MotorsDirection.DOWN_RIGHT)
            waitUntilMotorsStop()
        elif key==N6_KEY:
            moveOneStep(MotorsDirection.RIGHT)
            waitUntilMotorsStop()
        elif key==N9_KEY:
            moveOneStep(MotorsDirection.UP_RIGHT)
            waitUntilMotorsStop()

        # Define/redefine ROI corner on Shift+Clic in image
        elif clic_pos is not None and clic_flags & cv2.EVENT_FLAG_SHIFTKEY:
            setAreaCorner(clic_pos)
            drawCurrentImage()
            setState(State.WAITING_AREA_DEFINITION)
            continue

        # Reset target pos
        elif key==DELETE_KEY:
            resetArea()
            drawCurrentImage()
            setState(State.WAITING_AREA_DEFINITION)

        # Toggle 'pause_after_each_step'
        elif key==P_KEY:
            pause_after_each_step = not pause_after_each_step
            print(f"pause_after_each_step: {pause_after_each_step}",flush=True)

        # Now, treat each state specifically
        elif state==State.WAITING_AREA_DEFINITION:
            # Refresh camera view every 10s
            if state_duration_s > 10.0 or key==SPACE_KEY:
                if isAreaSet():
                    before_sun_move_img = current_img
                    setState(State.WAITING_SUN_MOVE)
                else:
                    current_img = cameraCapture()
                    drawCurrentImage()
                    setState(State.WAITING_AREA_DEFINITION)

        elif state==State.WAITING_SUN_MOVE:
            # Start tracking every 60s
            if state_duration_s > 60.0 or key==SPACE_KEY:
                if CAPTURE_ONLY_AREA_IN_TRACKING:
                    area_left_px, area_top_px, area_right_px, area_bottom_px = getArea()
                    area_img = cameraCaptureArea(area_left_px, area_top_px, area_right_px, area_bottom_px)
                    current_img[area_top_px:area_bottom_px+1, area_left_px:area_right_px+1] = area_img
                else:
                    current_img = cameraCapture()
                if not startTracking(before_sun_move_img, current_img):
                    setState(State.WAITING_SUN_MOVE)
                elif pause_after_each_step:
                    setState(State.TRACKING_PAUSED)
                else:
                    setState(State.TRACKING)

        elif state==State.TRACKING:
            # Check motors every 1s
            if state_duration_s > 1.0:
                status = getMotorsStatus()
                if status==MotorsStatus.MOVING_ONE_STEP:
                    # wait for the motors to finish the move
                    setState(State.TRACKING)
                elif status==MotorsStatus.LOCKED:
                    if CAPTURE_ONLY_AREA_IN_TRACKING:
                        area_left_px, area_top_px, area_right_px, area_bottom_px = getArea()
                        area_img = cameraCaptureArea(area_left_px, area_top_px, area_right_px, area_bottom_px)
                        current_img[area_top_px:area_bottom_px+1, area_left_px:area_right_px+1] = area_img
                    else:
                        current_img = cameraCapture()
                    if updateTracking(current_img):
                        # TODO replace by captureColor() in any case
                        if CAPTURE_ONLY_AREA_IN_TRACKING:
                            current_img = cameraCapture()
                            drawCurrentImage()
                        saveLastColorCapture("target")
                        setState(State.WAITING_SUN_MOVE)
                    elif pause_after_each_step:
                        setState(State.TRACKING_PAUSED)
                    else:
                        setState(State.TRACKING)

                else:
                    raise Exception(f"Incorrect motors status: {status}")

        elif state==State.TRACKING_PAUSED:
            if key==SPACE_KEY:
                setState(State.TRACKING)

        else:
            raise Exception(f"Unknwon state: {state}")

except Exception as exception:
    print("FATAL EXCEPTION:",flush=True)
    print(exception,flush=True)
    traceback.print_exc()
    cv2.waitKey(0)
