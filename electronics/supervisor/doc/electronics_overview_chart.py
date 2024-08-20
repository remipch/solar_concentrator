# This script builds the overview chart used in the component README

# requirements:
# pip install drawsvg
# pip install httpimport

import httpimport

with httpimport.github_repo('remipch', 'svg_chart', ref='26eeccf6c8ebdba7bfa6ac5d9352b09bc792b5ac'):
  from svg_chart import *

chart = Chart(node_width=230,cluster_margin=25)

battery = Node(chart, 0, -2, "12V Battery")
board = Node(chart, 0, 0, "Supervisor Board")
driver_a = Node(chart, -1, 2, "Motor Driver A")
driver_b = Node(chart, 1, 2, "Motor Driver B")
panel_a = Node(chart, -1, 4, "Panel A")
panel_b = Node(chart, 1, 4, "Panel B")

Edge(chart, battery, driver_a, "->")
Edge(chart, battery, board, "->")
Edge(chart, board, driver_a, "->")
Edge(chart, board, driver_b, "->")
Edge(chart, battery, driver_b, "->")
Edge(chart, driver_a, panel_a, "->")
Edge(chart, driver_b, panel_b, "->")

Cluster(chart, [board, driver_a, driver_b], "Electronic Case")

chart.exportSvg("electronics_overview_chart.svg")
