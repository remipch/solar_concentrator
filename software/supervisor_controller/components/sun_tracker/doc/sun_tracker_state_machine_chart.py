# This script builds the state machine charts used in the component README

# requirements:
# pip install drawsvg
# pip install httpimport

import httpimport

with httpimport.github_repo('remipch', 'svg_chart', ref='26eeccf6c8ebdba7bfa6ac5d9352b09bc792b5ac'):
  from svg_chart import *

chart = Chart(node_width=200)

uninitialized = Node(chart, 1, -1.5, "UNINITIALIZED", shape=NodeShape.ROUNDED_RECTANGLE, color="#bcd7ff")
idle = Node(chart, 1, 0, "IDLE", shape=NodeShape.ROUNDED_RECTANGLE, color="#bcd7ff")
idle_back = Point(chart, 2,0)
start_cond_1 = Node(chart, 1, 2, shape=NodeShape.DIAMOND)
result_error = Point(chart, 2.5, 2)
start_cond_2 = Node(chart, 1, 4, shape=NodeShape.DIAMOND)
result_success = Point(chart, 3, 4)
tracking = Node(chart, 1, 6, "TRACKING", shape=NodeShape.ROUNDED_RECTANGLE, color="#bcd7ff")
tracking_back = Point(chart, 0.2, 6)
stopping = Node(chart, 3.2, 6, "STOPPING", shape=NodeShape.ROUNDED_RECTANGLE, color="#bcd7ff")

Edge(chart, uninitialized, idle, "->")
Edge(chart, idle, start_cond_1, "->", "START / sun_tracker_logic_detect()")
Edge(chart, idle_back, idle, "->")
Edge(chart, start_cond_1, result_error, "-", "[else] / result=ERROR")
Edge(chart, start_cond_1, start_cond_2, "->", "[detection.result==SUCCESS]")
Edge(chart, start_cond_2, result_success, "-", "[detection.direction==NONE] / result=SUCCESS")
Edge(chart, start_cond_2, tracking, "->", "[else] / motors_start_move(direction)")
Edge(chart, tracking, stopping, "->", "STOP")
Edge(chart, stopping, idle_back, "-", "MOTORS_STOPPED / result=ABORTED", layout=EdgeLayout.RIGHT_RIGHT_CURVED)
Edge(chart, result_success, idle_back, "-", layout=EdgeLayout.RIGHT_RIGHT_CURVED)
Edge(chart, result_error, idle_back, "-", layout=EdgeLayout.RIGHT_RIGHT_CURVED)
Edge(chart, tracking, tracking_back, "-")
Edge(chart, tracking_back, start_cond_1, "->", "MOTORS_STOPPED / sun_tracker_logic_detect()", layout=EdgeLayout.LEFT_LEFT_CURVED)

# Manually adjust view size because edge text is not yet automatically taken into account
Point(chart, 5.1, 5)
Point(chart, -1.1, 0)

chart.exportSvg("sun_tracker_state_machine.svg")
