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

current_img = None

# user actions
ESCAPE_KEY = 27     # quit progam
SPACE_KEY = 32      # skip a waiting state
ENTER_KEY = 13      # set target pos (to the last clicked position in image)
DELETE_KEY = 255    # reset target pos

# print and flush
def prn(text):
    print(text,flush=True)

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

    # Now, treat each state specifically
    if state==State.WAITING_TARGET_DEFINITION:
        # Refresh camera view every 5s
        if state_duration_s > 5.0:
            captureAndShow()
            setState(State.WAITING_TARGET_DEFINITION)
            continue

    elif state==State.WAITING_SUN_MOVE:
        # Refresh camera view every 5s
        if state_duration_s > 5.0:
            captureAndShow()
            setState(State.WAITING_SUN_MOVE)
            continue

    # At this point the state must have been treated
    else:
        raise Exception(f"Unknwon state: {state}")

