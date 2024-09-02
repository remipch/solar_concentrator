# Copyright (C) 2024 Rémi Peuchot
# This code is distributed under GNU GPL v3 license

from panda3d.core import *
from direct.showbase.ShowBase import ShowBase
from settings_panel import SettingsPanel
from direct.task import Task
from constants import *

import zmq

# Note : because two cameras are successively involved in the full process of this application,
# power measure needs two rendering iterations to be computed (2 standard panda igLoop tasks)
#  - remote_control_server changes something in settings
#  - nodes are updated accordingly
#  - sun is rendered on mirrors 'measure_surface' (FIRST required igLoop iteration)
#  - mirrors 'measure_camera' record 'measure_surface' in 'measure_texture'
#  - mirrors sun_reflection_projector project 'measure_texture' on reflection receiver
#  - sun reflection are rendered on reflection receiver (SECOND required igLoop iteration)
#  - measure_area 'measure_camera' record measure_rectangle and count power
# So we define the following constant to count down the panda loop execution between the request reception and the measure reply
PANDA_LOOP_REQUIRED = 2


class RemoteControlServer:
    def __init__(self, settings):
        self.settings = settings
        self.context = zmq.Context()
        self.socket = self.context.socket(zmq.REP)
        self.socket.bind("tcp://*:" + REMOTE_CONTROL_PORT)
        self.asked_measures = None
        self.waiting_panda_loop_count = 0

        # sort=-5 allows to update parameters
        # - after dataLoop Task wich control keyboard and mouse
        # - before eventManager system Task
        taskMgr.add(self.remoteRequestReceiverTask,
                    "remoteRequestReceiver", sort=-5)

        # sort=70 allow to send reply after all events have been treated
        # (display has been updated so we send value corresponding to the request values)
        taskMgr.add(self.remoteReplySenderTask, "remoteReplySender", sort=70)

    def remoteRequestReceiverTask(self, task):
        if self.socket.poll(zmq.NOBLOCK) != 0:
            request = self.socket.recv_json()

            # Set all settings
            for setting_identifier_and_value in request["setting_values"].items():
                setting_identifier, setting_value = setting_identifier_and_value
                self.settings.setSettingValue(
                    setting_identifier, setting_value)

            # Keep the reply, it will be sent after all the events have been computed
            self.asked_measures = request["asked_measures"]

            # Start standard panda loop count down
            self.waiting_panda_loop_count = PANDA_LOOP_REQUIRED

        return Task.cont

    def remoteReplySenderTask(self, task):
        # Standard panda loop count down
        if self.waiting_panda_loop_count > 0:
            self.waiting_panda_loop_count = self.waiting_panda_loop_count - 1

        # All standard panda loop done
        if self.asked_measures is not None and self.waiting_panda_loop_count == 0:
            # Get all asked measures
            measure_values = {}
            for asked_measure in self.asked_measures:
                measure_values[asked_measure] = self.settings.getMeasureValue(
                    asked_measure
                )
            #  Send reply back to client
            self.socket.send_json(measure_values)
            self.asked_measures = None

        return Task.cont
