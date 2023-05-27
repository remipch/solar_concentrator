from enum import Enum

# WARNING : string values must match web api direction
class MotorsDirection(str, Enum):
    UP         = "up"
    UP_RIGHT   = "up-right"
    RIGHT      = "right"
    DOWN_RIGHT = "down-right"
    DOWN       = "down"
    DOWN_LEFT  = "down-left"
    LEFT       = "left"
    UP_LEFT    = "up-left"
