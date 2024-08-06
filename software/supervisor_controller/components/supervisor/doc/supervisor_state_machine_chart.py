# This script builds the state machine charts used in the component README

# requirements:
# pip install drawsvg
# pip install httpimport

import httpimport

with httpimport.github_repo('remipch', 'svg_chart', ref='26eeccf6c8ebdba7bfa6ac5d9352b09bc792b5ac'):
  from svg_chart import *

chart = Chart(node_width=230)

uninitialized = Node(chart, 0.2, -2, "UNINITIALIZED", shape=NodeShape.ROUNDED_RECTANGLE, color="#bcd7ff")
idle = Node(chart, 0, 0, "IDLE", shape=NodeShape.ROUNDED_RECTANGLE, color="#bcd7ff")
idle_aborted = Point(chart, 1, 0)
idle_stop = Point(chart, 3, 0)
manual_moving = Node(chart, -1, 3, "MANUAL_MOVING", shape=NodeShape.ROUNDED_RECTANGLE, color="#bcd7ff")
stop_manual = Point(chart, -1.7, 3)
start_manual = Point(chart, -1.9, 2.5)
sun_tracking = Node(chart, 1, 3, "SUN_TRACKING", shape=NodeShape.ROUNDED_RECTANGLE, color="#bcd7ff")
stop_tracking = Point(chart, 0.3, 3)
waiting_sun_move = Node(chart, 2.5, 3, "WAITING_SUN_MOVE", shape=NodeShape.ROUNDED_RECTANGLE, color="#bcd7ff")
error = Node(chart, 1.2, 5.5, "ERROR", shape=NodeShape.ROUNDED_RECTANGLE, color="#bcd7ff")

Edge(chart, manual_moving, stop_manual, "-", layout=EdgeLayout.BOTTOM_BOTTOM_CURVED)
Edge(chart, manual_moving, start_manual, "-", layout=EdgeLayout.BOTTOM_BOTTOM_CURVED)
Edge(chart, stop_manual, manual_moving, "->", "STOP", layout=EdgeLayout.TOP_TOP_CURVED)
Edge(chart, start_manual, manual_moving, "->", "START_MANUAL_MOVE", layout=EdgeLayout.TOP_TOP_CURVED)
Edge(chart, manual_moving, idle, "->", "MOTORS_STOPPED", layout=EdgeLayout.TOP_TOP_CURVED)
Edge(chart, idle, manual_moving, "->", "START_MANUAL_MOVE", layout=EdgeLayout.TOP_BOTTOM_CURVED)
Edge(chart, uninitialized, idle, "->")
Edge(chart, sun_tracking, stop_tracking, "-", layout=EdgeLayout.BOTTOM_BOTTOM_CURVED)
Edge(chart, stop_tracking, sun_tracking, "->", "STOP", layout=EdgeLayout.TOP_TOP_CURVED)
Edge(chart, manual_moving, sun_tracking, "->", "START_SUN_TRACKING", layout=EdgeLayout.BOTTOM_BOTTOM_CURVED)
Edge(chart, idle, sun_tracking, "->", "START_SUN_TRACKING", layout=EdgeLayout.TOP_BOTTOM_CURVED)
Edge(chart, idle_aborted, idle, "->")
Edge(chart, idle_stop, idle_aborted, "-")
Edge(chart, waiting_sun_move, idle_aborted, "-", "STOP", layout=EdgeLayout.RIGHT_RIGHT_CURVED)
Edge(chart, sun_tracking, idle_aborted, "-", "ABORTED", layout=EdgeLayout.RIGHT_RIGHT_CURVED)
Edge(chart, sun_tracking, waiting_sun_move, "->", "SUCCESS")
Edge(chart, sun_tracking, error, "->", "ERROR")
Edge(chart, waiting_sun_move, sun_tracking, "->", "TIMEOUT", layout=EdgeLayout.BOTTOM_BOTTOM_CURVED)
Edge(chart, error, idle_stop, "-", "STOP", layout=EdgeLayout.RIGHT_RIGHT_CURVED)

chart.exportSvg("supervisor_state_machine.svg")
