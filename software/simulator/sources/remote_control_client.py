# Copyright (C) 2024 Rémi Peuchot
# This code is distributed under GNU GPL v3 license (see software/LICENSE.md)

from constants import *
import zmq


class RemoteControlClient:
    def __init__(self):
        print("Connecting to RemoteControlServer…")
        self.context = zmq.Context()
        self.socket = self.context.socket(zmq.REQ)
        self.socket.connect("tcp://localhost:" + REMOTE_CONTROL_PORT)
        # Define 10s timeout :
        # - let time to the application to start if it has just been launched
        # - give a timeout to the client if the app crash (instead of waiting forever)
        self.socket.setsockopt(zmq.RCVTIMEO, 30000)

        # Send a first empty request to wait the application to be ready
        self.request({}, [])

    # send a request to remote server
    # setting_values is a dictionary containing parameter identifier and corresponding values :
    #   {"settings_identifier" : settings_value , ...}
    # asked_measures is an array with measure identifier we want in the reply :
    #   ["measure_identifier_1","measure_identifier_2",...]
    # returns a dictionary with measure identifiers and values :
    #   {"measure_identifier_1":value_1,"measure_identifier_2":value_2}
    def request(self, setting_values, asked_measures):
        request = {"setting_values": setting_values,
                   "asked_measures": asked_measures}
        self.socket.send_json(request)
        print(f"Request sent with settings:\n{setting_values}", flush=True)

        reply = self.socket.recv_json()
        print(f"Reply received:\n{reply}", flush=True)

        return reply
