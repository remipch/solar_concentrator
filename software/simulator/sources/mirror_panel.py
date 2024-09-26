# Copyright (C) 2024 Rémi Peuchot (https://remipch.github.io/)
# This code is distributed under GNU GPL v3 license

from panda3d.core import *
import sys
from direct.showbase.ShowBase import ShowBase
from solar_mirror import SolarMirror
from sun_follower import SunFollower
from direct.task import Task
from constants import *
from math import degrees, atan, tan, sqrt

STAND_SPAN = 0.3

if MULTI_MIRROR_ENABLED:
    # row_count*column_count must be <= 14 (see top comment in solar_mirror)
    MIRROR_ROW_COUNT = 4
    MIRROR_COLUMN_COUNT = 3
else:
    MIRROR_ROW_COUNT = 1
    MIRROR_COLUMN_COUNT = 1

FOCAL_LENGTH = 10  # distance where sun rays cross
PARABOLE_AMPLITUDE = 1/(4*FOCAL_LENGTH)


class MirrorPanel:
    def __init__(
        self,
        parent_np,
        sun_light_np,
        reflection_receiver_np,
        target_np,
        mirror_camera_bitmask,
        settings,
    ):
        self.sun_light_np = sun_light_np
        self.target_np = target_np
        self.settings = settings

        # Create root node path
        self.panel_np = parent_np.attachNewNode("mirror_panel")

        lines = LineSegs("stand")
        lines.setColor(0, 0, 0, 1)
        lines.moveTo(-STAND_SPAN, 0, 0)
        lines.drawTo(0, 0, 1)
        lines.drawTo(0, -STAND_SPAN, 0)
        lines.moveTo(STAND_SPAN, 0, 0)
        lines.drawTo(0, 0, 1)
        lines.setThickness(2)
        self.stand_np = self.panel_np.attachNewNode(lines.create())

        self.orientable_frame_np = self.panel_np.attachNewNode(
            "orientable_frame")

        lines = LineSegs("orientable_frame_wires")
        lines.setColor(0, 0, 0, 1)
        lines.moveTo(-0.5, 0.98, -0.5)
        lines.drawTo(0, 0, 0)
        lines.drawTo(-0.5, 0.98, 0.5)
        lines.moveTo(0.5, 0.98, -0.5)
        lines.drawTo(0, 0, 0)
        lines.drawTo(0.5, 0.98, 0.5)
        lines.setThickness(2)
        self.orientable_frame_wires_np = self.orientable_frame_np.attachNewNode(
            lines.create()
        )

        # Prevent the lines to project sun shadow
        # (https://discourse.panda3d.org/t/unwanted-nodes-are-casting-shadows-even-after-subnode-setautoshader/26052/2)
        self.orientable_frame_wires_np.hide(SUN_LIGHT_BITMASK)

        self.mirror_center_np = self.orientable_frame_np.attachNewNode(
            "mirror_center")

        self.mirrors = []
        for row in range(MIRROR_ROW_COUNT):
            current_row = []
            for col in range(MIRROR_COLUMN_COUNT):
                mirror_camera_bitmask = mirror_camera_bitmask << 1
                mirror = SolarMirror(
                    self.mirror_center_np,
                    sun_light_np,
                    reflection_receiver_np,
                    target_np,
                    mirror_camera_bitmask,
                )
                # mirror pose will be set by 'enableParabolicMirrors'
                current_row.append(mirror)
            self.mirrors.append(current_row)

        self.sun_following_enabled = False
        self.sun_follower = SunFollower(self, target_np)

        # Constants added to orientation to simulate errors
        self.mirror_head = 0
        self.mirror_pitch = 0
        self.mirror_head_offset = 0
        self.mirror_pitch_offset = 0

        # sort=25 allows to update panel orientation after input/output events
        # and before igloop system Task which draws the scene
        taskMgr.add(self.sunFollowingTask, "sunFollowing", sort=25)

    def getNodePath(self):
        return self.panel_np

    def getMainMirror(self):
        return self.mirrors[int(MIRROR_ROW_COUNT/2)][int(MIRROR_COLUMN_COUNT/2)]

    def setStandHeight(self, height):
        self.stand_np.setSz(height)
        self.orientable_frame_np.setZ(height)

    def setMirrorWidth(self, width):
        self.orientable_frame_np.setSx(width)

    def setMirrorHeight(self, height):
        self.orientable_frame_np.setSz(height)

    def setRotationRadius(self, radius):
        self.orientable_frame_wires_np.setSy(radius)
        self.mirror_center_np.setY(radius)

    def setOrientationOffset(self, head, pitch):
        self.mirror_head_offset = head
        self.mirror_pitch_offset = pitch

    def enableSunFollowing(self, sun_following_enabled):
        self.sun_following_enabled = sun_following_enabled

    def enableParabolicMirrors(self, parabolic_mirrors_enabled):
        for row in range(MIRROR_ROW_COUNT):
            for col in range(MIRROR_COLUMN_COUNT):
                mirror = self.mirrors[row][col]
                mirror_np = mirror.getNodePath()
                x = col - (MIRROR_COLUMN_COUNT-1)/2
                z = row - (MIRROR_ROW_COUNT-1)/2
                mirror_np.setPos(x, 0, z)
                if parabolic_mirrors_enabled:
                    d = sqrt(x**2+z**2)  # distance from parabole center
                    y = PARABOLE_AMPLITUDE*d**2
                    mirror_np.setY(y)
                    alpha = atan(2*PARABOLE_AMPLITUDE*d)
                    if alpha != 0:
                        yd = y + d / tan(alpha)
                        mirror_np.lookAt(
                            self.mirror_center_np, 0, yd, 0)
                else:
                    mirror_np.lookAt(self.mirror_center_np,
                                     0, 2*FOCAL_LENGTH, 0)

    # return head, pitch pair

    def getMirrorOrientation(self):
        return self.mirror_head, self.mirror_pitch

    def setMirrorOrientationWithoutOffset(self, mirror_head, mirror_pitch):
        self.mirror_head = mirror_head
        self.mirror_pitch = mirror_pitch
        self.orientable_frame_np.setHpr(mirror_head, mirror_pitch, 0)

    def setMirrorOrientationWithOffset(self, mirror_head, mirror_pitch):
        self.mirror_head = mirror_head
        self.mirror_pitch = mirror_pitch
        self.orientable_frame_np.setHpr(
            mirror_head + self.mirror_head_offset,
            mirror_pitch + self.mirror_pitch_offset,
            0,
        )

    def setDirectionLinesAlpha(self, alpha):
        for row in range(MIRROR_ROW_COUNT):
            for col in range(MIRROR_COLUMN_COUNT):
                mirror = self.mirrors[row][col]
                mirror.setDirectionLinesAlpha(alpha)

    def sunFollowingTask(self, task):
        if self.settings.settingsHaveChanged():
            if (
                self.sun_following_enabled
                and self.settings.getMeasureValue(
                    "sun_direct_insolation_in_watt_per_square_meter"
                )
                > 0
            ):
                self.sun_follower.minimizeProjectionDistanceFromTarget()
            else:
                self.orientable_frame_np.setHpr(
                    self.mirror_head + self.mirror_head_offset,
                    self.mirror_pitch + self.mirror_pitch_offset,
                    0,
                )

        return Task.cont
