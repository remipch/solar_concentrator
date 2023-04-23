from enum import Enum
import time

# manage supervisor state


class State(str, Enum):
    UNDEFINED                   = "UNDEFINED"
    WAITING_TARGET_DEFINITION   = "WAITING_TARGET_DEFINITION"
    WAITING_SUN_MOVE            = "WAITING_SUN_MOVE"
    TRACKING                    = "TRACKING"
    TRACKING_PAUSED             = "TRACKING_PAUSED"


current_state = State.UNDEFINED

# time when current state started
# in floating point seconds
state_start_time = time.monotonic()

def setState(state):
    global current_state, state_start_time
    current_state = state
    state_start_time = time.monotonic()
    print(f"current_state: {state} (at {state_start_time} s)",flush=True)

def getState():
    state_duration = time.monotonic() - state_start_time
    return current_state, state_duration
