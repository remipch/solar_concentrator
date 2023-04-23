from enum import Enum

class MotorsDirection(str, Enum):
    UP_RIGHT   = "up_right"
    RIGHT      = "right"
    DOWN_RIGHT = "down_right"
    DOWN_LEFT  = "down_left"
    LEFT       = "left"
    UP_LEFT    = "up_left"
