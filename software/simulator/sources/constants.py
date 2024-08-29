# Copyright (C) 2024 Rémi Peuchot
# This code is distributed under GNU GPL v3 license (see software/LICENSE.md)

from panda3d.core import BitMask32
import os

# Bitmask used across the whole app
MAIN_CAMERA_BITMASK = BitMask32.bit(1)
SUN_LIGHT_BITMASK = BitMask32.bit(2)
MEASURE_CAMERA_BITMASK = BitMask32.bit(3)
FIRST_MIRROR_CAMERA_BITMASK = BitMask32.bit(4)

# Note : sun light blue level is used to represent light power reflected by mirrors
# -> keep SUN_LIGHT_COLOR with SUN_LIGHT_BLUE_LEVEL blue level
SUN_LIGHT_BLUE_LEVEL = 0.2
SUN_LIGHT_COLOR = (0.25, 0.25, SUN_LIGHT_BLUE_LEVEL, 1)

REMOTE_CONTROL_PORT = "5555"

LOCAL_PATH = os.path.dirname(os.path.abspath(__file__))
MODELS_PATH = os.path.join(LOCAL_PATH, "../models/")
SOURCES_PATH = os.path.join(LOCAL_PATH, "../sources/")

# TODO define and document all task priorities here

# If True: create only one panel with several fixed mirrors in parabolic shape
# If False: create several panel, each one contains only one plan mirror
MULTI_MIRROR_ENABLED = False
