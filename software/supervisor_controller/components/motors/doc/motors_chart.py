# This script builds the main component diagram used in the component README

# requirements:
# pip install drawsvg
# pip install httpimport

import httpimport

with httpimport.github_repo('remipch', 'svg_chart', ref='26eeccf6c8ebdba7bfa6ac5d9352b09bc792b5ac'):
  from svg_chart import *

chart = Chart(node_width=260,cluster_margin=20)

motors = Node(chart, 0, 0, "motors")
motors_state_machine = Node(chart, 0.5, 1.5, "motors_state_machine")
motors_hw = Node(chart, 1, 3, "motors_hw")
motors_direction = Node(chart, 0, 4.5, "motors_direction")

esp_event = Node(chart, 2.2, 0, "esp_event")
esp_timer = Node(chart, 2.2, 1.5, "esp_timer")
uart = Node(chart, 2.2, 3, "driver/uart")

Edge(chart, motors, motors_direction, "-->")
Edge(chart, motors, motors_state_machine, "-->")
Edge(chart, motors_state_machine, motors_direction, "-->")
Edge(chart, motors_state_machine, motors_hw, "-->")
Edge(chart, motors_hw, motors_direction, "-->")

Edge(chart, motors, esp_event, "-->")
Edge(chart, motors, esp_timer, "-->", layout=EdgeLayout.LEFT_RIGHT_STRAIGHT)
Edge(chart, motors_hw, uart, "-->")

Cluster(chart, [motors, motors_state_machine, motors_hw, motors_direction], "component/motors")
Cluster(chart, [esp_event, esp_timer, uart], "esp-idf")

chart.exportSvg("motors.svg")
