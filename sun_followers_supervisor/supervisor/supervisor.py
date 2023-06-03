# main supervisor script, just run it

import cv2
import numpy as np
import math
from panel_web_access import *
from user_interface import *
from state import *
from tracking2 import *

pause_after_each_step = True

CAPTURE_ONLY_AREA_IN_TRACKING = True

# user actions
ESCAPE_KEY = 27     # quit progam
SPACE_KEY = 32      # skip a waiting state
DELETE_KEY = 255    # reset target pos
A_KEY = 97          # is_morning = False
M_KEY = 109         # is_morning = True
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
    drawArea(current_img)

# initialisation
current_img = cameraCapture()
drawCurrentImage()
setState(State.WAITING_AREA_DEFINITION)
setMorning(True) # TODO : update in WAITING_SUN_MOVE state according to time change

def treatManualMove(key):
    if key==N8_KEY:
        moveOneStep(MotorsDirection.UP)
        setState(State.MANUAL_MOVE)
    elif key==N1_KEY:
        moveOneStep(MotorsDirection.DOWN_LEFT)
        setState(State.MANUAL_MOVE)
    elif key==N4_KEY:
        moveOneStep(MotorsDirection.LEFT)
        setState(State.MANUAL_MOVE)
    elif key==N7_KEY:
        moveOneStep(MotorsDirection.UP_LEFT)
        setState(State.MANUAL_MOVE)
    elif key==N2_KEY:
        moveOneStep(MotorsDirection.DOWN)
        setState(State.MANUAL_MOVE)
    elif key==N3_KEY:
        moveOneStep(MotorsDirection.DOWN_RIGHT)
        setState(State.MANUAL_MOVE)
    elif key==N6_KEY:
        moveOneStep(MotorsDirection.RIGHT)
        setState(State.MANUAL_MOVE)
    elif key==N9_KEY:
        moveOneStep(MotorsDirection.UP_RIGHT)
        setState(State.MANUAL_MOVE)

# application main loop
try:
    while True:
        key, clic_pos, clic_flags = updateUserInterface()
        state, state_duration_s = getState()

        # Treat common key press (meaningful for all states)

        # Exit application on escape
        if key==ESCAPE_KEY:
            exit(0)

        # Define/redefine ROI corner on Shift+Clic in image
        elif clic_pos is not None and clic_flags & cv2.EVENT_FLAG_SHIFTKEY:
            setAreaCorner(clic_pos)
            drawCurrentImage()
            if isAreaSet():
                setState(State.WAITING_SUN_MOVE)
            else:
                setState(State.WAITING_AREA_DEFINITION)
            continue

        # Reset target pos
        elif key==DELETE_KEY:
            resetArea()
            drawCurrentImage()
            setState(State.WAITING_AREA_DEFINITION)

        # Afternoon
        elif key==A_KEY:
            setMorning(False)
            drawCurrentImage()

        # Morning
        elif key==M_KEY:
            setMorning(True)
            drawCurrentImage()

        # Toggle 'pause_after_each_step'
        elif key==P_KEY:
            pause_after_each_step = not pause_after_each_step
            print(f"pause_after_each_step: {pause_after_each_step}",flush=True)

        # Now, treat each state specifically
        elif state==State.WAITING_AREA_DEFINITION:
            # Refresh camera view every 10s
            if state_duration_s > 10.0 or key==SPACE_KEY:
                current_img = cameraCapture()
                drawCurrentImage()
                setState(state)
            else:
                treatManualMove(key)

        elif state==State.MANUAL_MOVE:
            if key==N0_KEY:
                stopMove()
                setState(State.MANUAL_MOVE)
            # Check motors every 1s
            elif state_duration_s > 1.0:
                status = getMotorsStatus()
                if status==MotorsStatus.MOVING_ONE_STEP:
                    # wait for the motors to finish the move
                    setState(State.MANUAL_MOVE)
                    continue
                elif status==MotorsStatus.LOCKED:
                    if isAreaSet():
                        setState(State.WAITING_SUN_MOVE)
                    else:
                        setState(State.WAITING_AREA_DEFINITION)
                else:
                    raise Exception(f"Incorrect motors status: {status}")

        elif state==State.WAITING_SUN_MOVE:
            # Start tracking every 60s
            if state_duration_s > 60.0 or key==SPACE_KEY:
                current_img = cameraCapture()
                drawCurrentImage()
                if startTracking(current_img):
                    setState(State.TRACKING)
                else:
                    setState(State.WAITING_SUN_MOVE)
            else:
                treatManualMove(key)

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
                    drawCurrentImage()
                    finished = updateTracking(current_img)
                    if finished:
                        print("TRACKING FINISHED",flush=True)
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
    cv2.waitKey(0)
