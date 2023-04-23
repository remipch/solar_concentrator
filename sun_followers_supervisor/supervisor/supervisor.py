# main supervisor script, just run it

import cv2
import numpy as np
import math
from panel_web_access import *
from user_interface import *
from state import *
from tracking import *

# defined by Ctrl+Clic on image
# reseted by 'Delete' key
target_pos = None

# tolerance arround target
target_tol_px = 20

current_img = None

pause_after_each_step = True

# user actions
ESCAPE_KEY = 27     # quit progam
SPACE_KEY = 32      # skip a waiting state
DELETE_KEY = 255    # reset target pos
P_KEY = 0         # toggle 'pause_after_each_step'

# print and flush
def prn(text):
    print(text,flush=True)

# todo : return img instead of global
def captureAndShow():
    global current_img

    current_img = cameraCapture()
    showDebugImage("current",current_img)
    if target_pos is not None:
        drawCross(target_pos[0],target_pos[1],target_tol_px,green)


# initialisation
captureAndShow()
setState(State.WAITING_TARGET_DEFINITION)

# application main loop
while True:
    key, clic_pos, clic_flags = updateUserInterface()
    state, state_duration_s = getState()

    # Treat common key press (meaningful for all states)

    # Exit application on escape
    if key==ESCAPE_KEY:
        exit(0)

    # Define/redefine target pos on Ctrl+Clic in image
    if clic_pos is not None and clic_flags & cv2.EVENT_FLAG_CTRLKEY:
        target_pos = clic_pos
        captureAndShow()
        setState(State.WAITING_SUN_MOVE)
        prn(f"New target defined: {target_pos}")
        continue

    # Reset target pos
    if key==DELETE_KEY:
        target_pos = None
        captureAndShow()
        setState(State.WAITING_TARGET_DEFINITION)
        continue

    # Toggle 'pause_after_each_step'
    if key==P_KEY:
        pause_after_each_step = not pause_after_each_step
        prn(f"pause_after_each_step: {pause_after_each_step}")
        continue

    # Now, treat each state specifically
    if state==State.WAITING_TARGET_DEFINITION:
        # Refresh camera view every 5s
        if state_duration_s > 5.0 or key==SPACE_KEY:
            captureAndShow()
            setState(State.WAITING_TARGET_DEFINITION)
            continue

    elif state==State.WAITING_SUN_MOVE:
        # Start tracking every 10s
        if state_duration_s > 10.0 or key==SPACE_KEY:
            captureAndShow()
            motors_direction = startTrackingOneStep(current_img, True)
            moveOneStep(motors_direction)
            setState(State.TRACKING)
            continue

    elif state==State.TRACKING:
        # Check motors every 1s
        if state_duration_s > 1.0 or key==SPACE_KEY:
            status = getMotorsStatus()
            if status==MotorsStatus.MOVING_ONE_STEP:
                # wait for the motors to finish the move
                setState(State.TRACKING)
                continue
            elif status==MotorsStatus.LOCKED:
                captureAndShow()
                error_px = finishTrackingOneStep(current_img, target_pos)
                if error_px > target_tol_px:
                    if pause_after_each_step:
                        setState(State.TRACKING_PAUSED)
                    else:
                        motors_direction = startTrackingOneStep(current_img, False)
                        setState(State.TRACKING)
                else:
                    setState(State.WAITING_SUN_MOVE)
                continue
            else:
                raise Exception(f"Incorrect motors status: {status}")

    elif state==State.TRACKING_PAUSED:
        if key==SPACE_KEY:
            captureAndShow()
            motors_direction = startTrackingOneStep(current_img, False)
            setState(State.TRACKING)

    else:
        raise Exception(f"Unknwon state: {state}")

