# main supervisor script, just run it

import cv2
import numpy as np
import math
from panel_web_access import *
from user_interface import *
from state import *
from tracking import *

pause_after_each_step = True

# user actions
ESCAPE_KEY = 27     # quit progam
SPACE_KEY = 32      # skip a waiting state
DELETE_KEY = 255    # reset target pos
P_KEY = 112           # toggle 'pause_after_each_step'
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
    drawTarget()
    drawRoi()

# initialisation
current_img = cameraCapture()
drawCurrentImage()
setState(State.WAITING_TARGET_DEFINITION)

# application main loop
while True:
    key, clic_pos, clic_flags = updateUserInterface()
    state, state_duration_s = getState()

    # Treat common key press (meaningful for all states)

    # Exit application on escape
    if key==ESCAPE_KEY:
        exit(0)

    # Define/redefine ROI corner on Shift+Clic in image
    elif clic_pos is not None and clic_flags & cv2.EVENT_FLAG_SHIFTKEY:
        setRoiCorner(clic_pos)
        resetTarget()
        drawCurrentImage()
        setState(State.WAITING_TARGET_DEFINITION)
        continue

    # Define/redefine target pos on Ctrl+Clic in image
    elif clic_pos is not None and clic_flags & cv2.EVENT_FLAG_CTRLKEY:
        setTarget(clic_pos)
        drawCurrentImage()
        if isTargetSet():
            setState(State.WAITING_SUN_MOVE)
        else:
            setState(State.WAITING_TARGET_DEFINITION)
        continue

    # Reset target pos
    elif key==DELETE_KEY:
        resetTarget()
        drawCurrentImage()
        setState(State.WAITING_TARGET_DEFINITION)

    # Toggle 'pause_after_each_step'
    elif key==P_KEY:
        pause_after_each_step = not pause_after_each_step
        print(f"pause_after_each_step: {pause_after_each_step}",flush=True)

    # Now, treat each state specifically
    elif state==State.WAITING_TARGET_DEFINITION:
        # Refresh camera view every 10s
        if state_duration_s > 10.0 or key==SPACE_KEY:
            current_img = cameraCapture()
            drawCurrentImage()
            setState(State.WAITING_TARGET_DEFINITION)
        elif key==N8_KEY:
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
                setState(State.WAITING_TARGET_DEFINITION)
            else:
                raise Exception(f"Incorrect motors status: {status}")

    elif state==State.WAITING_SUN_MOVE:
        # Start tracking every 60s
        if state_duration_s > 60.0 or key==SPACE_KEY:
            current_img = cameraCapture()
            drawCurrentImage()
            startTrackingOneStep(current_img, True)
            setState(State.TRACKING)

    elif state==State.TRACKING:
        # Check motors every 1s
        if state_duration_s > 1.0:
            status = getMotorsStatus()
            if status==MotorsStatus.MOVING_ONE_STEP:
                # wait for the motors to finish the move
                setState(State.TRACKING)
            elif status==MotorsStatus.LOCKED:
                current_img = cameraCapture()
                drawCurrentImage()
                target_reached = finishTrackingOneStep(current_img)
                if target_reached:
                    print("TARGET REACHED",flush=True)
                    saveLastColorCapture("target")
                    setState(State.WAITING_SUN_MOVE)
                else:
                    if pause_after_each_step:
                        setState(State.TRACKING_PAUSED)
                    else:
                        startTrackingOneStep(current_img, False)
                        setState(State.TRACKING)
            else:
                raise Exception(f"Incorrect motors status: {status}")

    elif state==State.TRACKING_PAUSED:
        if key==SPACE_KEY:
            startTrackingOneStep(current_img, False)
            setState(State.TRACKING)

    else:
        raise Exception(f"Unknwon state: {state}")
