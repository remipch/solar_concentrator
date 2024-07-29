# This script builds the state machine charts used in the component README

# requirements:
# pip install drawsvg
# pip install httpimport

import httpimport

with httpimport.github_repo('remipch', 'svg_chart', ref='6152ae1866b5ac7a4d404108f513185c9b10ad23'):
  from svg_chart import *

chart = Chart(node_width=200)

c1 = -1
c2 = 1

uninitialized = Node(chart, 0, 1, "UNINITIALIZED", shape=NodeShape.ROUNDED_RECTANGLE, color="#bcd7ff")
init = Node(chart, 0, 2.5, "hw_init()")
init_cond = Node(chart, 0, 4, shape=NodeShape.DIAMOND)
error = Node(chart, -3, 4, "ERROR", shape=NodeShape.ROUNDED_RECTANGLE, color="#bcd7ff")
stopped = Node(chart, 0, 6, "STOPPED", shape=NodeShape.ROUNDED_RECTANGLE, color="#bcd7ff")
start_move = Node(chart, 0, 8, "hw_start_move()")
moving = Node(chart, c1, 10, "MOVING", shape=NodeShape.ROUNDED_RECTANGLE, color="#bcd7ff")
stopping = Node(chart, c2, 10, "STOPPING", shape=NodeShape.ROUNDED_RECTANGLE, color="#bcd7ff")
get_state_1 = Node(chart, c1, 11.5, "hw_get_state()")
state_cond_1 = Node(chart, c1, 13, shape=NodeShape.DIAMOND)
get_state_2 = Node(chart, c2, 11.5, "hw_get_state()")
state_cond_2 = Node(chart, c2, 13, shape=NodeShape.DIAMOND)

# Note: edges order creation is used to order multiple edges appearing on the same node border
Edge(chart, uninitialized, init, "->")
Edge(chart, init, init_cond, "->")
Edge(chart, init_cond, error, "-->", "(else)")
Edge(chart, init_cond, stopped, "->", "(init_result==NO_ERROR)")
Edge(chart, moving, start_move, "->", "START_MOVE", layout=EdgeLayout.LEFT_LEFT_CURVED)
Edge(chart, stopped, start_move, "->", "START_MOVE")
Edge(chart, stopping, start_move, "->", "START_MOVE", layout=EdgeLayout.RIGHT_RIGHT_CURVED)
Edge(chart, start_move, moving, "->", layout=EdgeLayout.TOP_BOTTOM_CURVED)
Edge(chart, moving, stopping, "->", "STOP")
Edge(chart, moving, get_state_1, "->")
Edge(chart, get_state_1, state_cond_1, "->")
Edge(chart, stopping, get_state_2, "->")
Edge(chart, get_state_2, state_cond_2, "->")
Edge(chart, state_cond_1, stopped, "->", "(state==STOPPED)", layout=EdgeLayout.LEFT_LEFT_CURVED)
Edge(chart, state_cond_2, stopped, "->", "(state==STOPPED)", layout=EdgeLayout.RIGHT_RIGHT_CURVED)
Edge(chart, state_cond_1, moving, "-->", "(else)", layout=EdgeLayout.RIGHT_RIGHT_CURVED)
Edge(chart, state_cond_2, stopping, "-->", "(else)", layout=EdgeLayout.LEFT_LEFT_CURVED)
Edge(chart, state_cond_2, error, "->", "(state==UNKNOWN)", layout=EdgeLayout.BOTTOM_BOTTOM_CURVED)
Edge(chart, state_cond_1, error, "->", "(state==UNKNOWN)", layout=EdgeLayout.BOTTOM_BOTTOM_CURVED)

# Manually adjust view size because edge text is not yet automatically taken into account
Point(chart, c2+1.6, 2)

chart.exportSvg("motors_state_machine.svg")
