# Copyright (C) 2024 Rémi Peuchot
# This code is distributed under GNU GPL v3 license

from direct.showbase.DirectObject import DirectObject
from panda3d.core import *
from direct.task import Task
from enum import Enum


class Mode(Enum):
    NONE = 1
    TRANSLATION = 2
    ROTATION = 3


# Custom camera mouse control
# - subjectively more inutitive than the default mouse control
# - inspired from :
# https://github.com/Schwarzbaer/panda_examples/blob/master/boilerplate/base.py
class CameraControl(DirectObject):
    def __init__(self):
        base.disableMouse()

        self.mode = Mode.NONE

        # Current camera state :
        self.camera_orbit = base.render.attach_new_node("camera_orbit")
        self.camera_pitch = self.camera_orbit.attach_new_node("camera_pitch")
        base.camera.reparent_to(self.camera_pitch)

        # Camera state stored when a new mode starts :
        self.origin_camera_orbit = base.render.attach_new_node(
            "origin_camera_orbit")
        self.origin_camera_pitch = 0
        self.origin_mouse_pos = None

        # Change mode :
        self.accept("mouse1", self.setMode, [Mode.ROTATION])
        self.accept("mouse1-up", self.setMode, [Mode.NONE])
        self.accept("mouse3", self.setMode, [Mode.TRANSLATION])
        self.accept("mouse3-up", self.setMode, [Mode.NONE])
        self.accept("wheel_up", self.moveCameraDistance, [-1])
        self.accept("wheel_down", self.moveCameraDistance, [1])
        base.taskMgr.add(self.cameraControlTask, "cameraControl")

        # Convenient initial setup
        self.camera_orbit.setPosHpr(-8, 5, 0, 20, 0, 0)
        self.camera_pitch.setP(-20)
        base.camera.setY(-50)

    def setMode(self, mode):
        if base.mouseWatcherNode.has_mouse() and self.mode == Mode.NONE:
            # Keep original positions when a new mode starts
            self.origin_camera_orbit.setPosHpr(
                self.camera_orbit, 0, 0, 0, 0, 0, 0)
            self.origin_camera_pitch = self.camera_pitch.getP()
            self.origin_mouse_pos = LPoint2f(base.mouseWatcherNode.get_mouse())
        self.mode = mode

    def moveCameraDistance(self, direction):
        base.camera.set_pos(base.camera.get_pos() * (1 + 0.1 * direction))

    def cameraControlTask(self, task):
        if base.mouseWatcherNode.has_mouse():
            mouse_pos = base.mouseWatcherNode.get_mouse()
            if self.mode == Mode.TRANSLATION:
                mouse_delta = mouse_pos - self.origin_mouse_pos
                self.camera_orbit.setX(
                    self.origin_camera_orbit, -50 * mouse_delta.getX()
                )
                self.camera_orbit.setY(
                    self.origin_camera_orbit, -50 * mouse_delta.getY()
                )
            elif self.mode == Mode.ROTATION:
                mouse_delta = mouse_pos - self.origin_mouse_pos
                self.camera_orbit.setH(
                    self.origin_camera_orbit, -180 * mouse_delta.getX()
                )
                new_pitch = self.origin_camera_pitch + 180 * mouse_delta.getY()
                self.camera_pitch.setP(min(max(new_pitch, -90), 90))
        return Task.cont
