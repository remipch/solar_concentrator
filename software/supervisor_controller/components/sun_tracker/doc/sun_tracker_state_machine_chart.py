# This script builds the state machine charts used in the component README

# requirements:
# pip install drawsvg
# pip install httpimport

import httpimport

with httpimport.github_repo('remipch', 'svg_chart', ref='6152ae1866b5ac7a4d404108f513185c9b10ad23'):
  from svg_chart import *

chart = Chart(node_width=200)

uninitialized = Node(chart, 1, -1.5, "UNINITIALIZED", shape=NodeShape.ROUNDED_RECTANGLE, color="#bcd7ff")
idle = Node(chart, 1, 0, "IDLE", shape=NodeShape.ROUNDED_RECTANGLE, color="#bcd7ff")
start_cond_1 = Node(chart, 1, 2, shape=NodeShape.DIAMOND)
result_error = Node(chart, 2, 2, "result = ERROR")
start_cond_2 = Node(chart, 1, 4, shape=NodeShape.DIAMOND)
result_success = Node(chart, 3, 4, "result = SUCCESS")
start_move = Node(chart, 1, 6, "start_move(direction)")
tracking = Node(chart, 1, 8, "TRACKING", shape=NodeShape.ROUNDED_RECTANGLE, color="#bcd7ff")
stopping = Node(chart, 3, 8, "STOPPING", shape=NodeShape.ROUNDED_RECTANGLE, color="#bcd7ff")

# Note: edges order creation is used to order multiple edges appearing on the same node border
Edge(chart, uninitialized, idle, "->")
Edge(chart, idle, start_cond_1, "->", "START")
Edge(chart, start_cond_1, result_error, "-->", "else")
Edge(chart, start_cond_1, start_cond_2, "->", "detection.result==SUCCESS")
Edge(chart, start_cond_2, result_success, "->", "detection.direction==NONE")
Edge(chart, start_cond_2, start_move, "-->", "else")
Edge(chart, start_move, tracking, "->")
Edge(chart, tracking, stopping, "->", "STOP")
Edge(chart, stopping, idle, "->", "MOTORS_STOPPED", layout=EdgeLayout.RIGHT_RIGHT_CURVED)
Edge(chart, result_success, idle, "->", layout=EdgeLayout.RIGHT_RIGHT_CURVED)
Edge(chart, result_error, idle, "->", layout=EdgeLayout.RIGHT_RIGHT_CURVED)
Edge(chart, tracking, start_cond_1, "->", "MOTORS_STOPPED", layout=EdgeLayout.LEFT_LEFT_CURVED)

# Manually adjust view size because edge text is not yet automatically taken into account
Point(chart, 4.8, 5)
Point(chart, -0.5, 0)

chart.exportSvg("sun_tracker_state_machine.svg")
