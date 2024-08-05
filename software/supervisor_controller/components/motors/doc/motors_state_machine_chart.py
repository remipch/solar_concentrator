# This script builds the state machine charts used in the component README

# requirements:
# pip install drawsvg
# pip install httpimport

import httpimport

with httpimport.github_repo('remipch', 'svg_chart', ref='26eeccf6c8ebdba7bfa6ac5d9352b09bc792b5ac'):
  from svg_chart import *

chart = Chart(node_width=200)

uninitialized = Node(chart, 0, 2.5, "UNINITIALIZED", shape=NodeShape.ROUNDED_RECTANGLE, color="#bcd7ff")
init_cond = Node(chart, 0, 4, shape=NodeShape.DIAMOND)
error = Node(chart, -2, 4, "ERROR", shape=NodeShape.ROUNDED_RECTANGLE, color="#bcd7ff")
stopped = Node(chart, 0, 6, "STOPPED", shape=NodeShape.ROUNDED_RECTANGLE, color="#bcd7ff")
moving = Node(chart, -0.5, 10, "MOVING", shape=NodeShape.ROUNDED_RECTANGLE, color="#bcd7ff")
stopping = Node(chart, 1, 10, "STOPPING", shape=NodeShape.ROUNDED_RECTANGLE, color="#bcd7ff")
state_cond_1 = Node(chart, -0.5, 11.5, shape=NodeShape.DIAMOND)
state_cond_2 = Node(chart, 1, 11.5, shape=NodeShape.DIAMOND)

# Note: edges order creation is used to order multiple edges appearing on the same node border
Edge(chart, uninitialized, init_cond, "->")
Edge(chart, init_cond, error, "->", "[else]")
Edge(chart, init_cond, stopped, "->", "[init_result==NO_ERROR]")
Edge(chart, stopped, moving, "->", "START_MOVE", layout=EdgeLayout.TOP_BOTTOM_CURVED)
Edge(chart, stopping, moving, "->", "START_MOVE", layout=EdgeLayout.TOP_TOP_CURVED)
Edge(chart, moving, stopping, "->", "STOP")
Edge(chart, moving, state_cond_1, "->")
Edge(chart, stopping, state_cond_2, "->")
Edge(chart, state_cond_1, stopped, "->", "[state==STOPPED]", layout=EdgeLayout.LEFT_LEFT_CURVED)
Edge(chart, state_cond_2, stopped, "->", "[state==STOPPED]", layout=EdgeLayout.RIGHT_RIGHT_CURVED)
Edge(chart, state_cond_1, moving, "->", "[else]", layout=EdgeLayout.RIGHT_RIGHT_CURVED)
Edge(chart, state_cond_2, stopping, "->", "[else]", layout=EdgeLayout.LEFT_LEFT_CURVED)
Edge(chart, state_cond_2, error, "->", "[state==UNKNOWN]", layout=EdgeLayout.BOTTOM_BOTTOM_CURVED)
Edge(chart, state_cond_1, error, "->", "[state==UNKNOWN]", layout=EdgeLayout.BOTTOM_BOTTOM_CURVED)

# Manually adjust view size because edge text is not yet automatically taken into account
Point(chart, 2.6, 2)

chart.exportSvg("motors_state_machine.svg")
