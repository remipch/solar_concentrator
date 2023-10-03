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

ESCAPE_KEY = 27     # quit progam

area = Rectangle(300,200,500,400)

# application main loop
try:
    while True:
        key, clic_pos, clic_flags = updateUserInterface()

        # Exit application on escape
        if key==ESCAPE_KEY:
            exit(0)

        current_img = cameraCaptureAndReplaceArea(area)
        showDebugImage("current",current_img,800,0,800,600)

except Exception as exception:
    print("FATAL EXCEPTION:",flush=True)
    print(exception,flush=True)
    traceback.print_exc()
    cv2.waitKey(0)
