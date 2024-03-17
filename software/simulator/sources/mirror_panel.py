from panda3d.core import *
import sys
from direct.showbase.ShowBase import ShowBase
from solar_mirror import SolarMirror
from sun_follower import SunFollower
from direct.task import Task
from constants import *

STAND_SPAN = 0.3

if MULTI_MIRROR_ENABLED:
    MIRROR_ROW_COUNT = 3
    MIRROR_COLUMN_COUNT = 3
    MIRROR_WIDTH = 0.3
    FOCAL_LENGTH = 20


class MirrorPanel:
    def __init__(
        self,
        parent_np,
        sun_light_np,
        reflection_receiver_np,
        target_np,
        mirror_camera_bitmask,
        settings,
        create_fake_spotlight,
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

        self.main_mirror = SolarMirror(
            self.orientable_frame_np,
            sun_light_np,
            reflection_receiver_np,
            target_np,
            mirror_camera_bitmask,
        )

        self.mirrors = []
        if MULTI_MIRROR_ENABLED:
            # Quick and ugly test :
            # TODO : focus point
            for row in range(MIRROR_ROW_COUNT):
                for col in range(MIRROR_COLUMN_COUNT):
                    if row != 1 or col != 1:
                        mirror_camera_bitmask = mirror_camera_bitmask << 1
                        mirror = SolarMirror(
                            self.main_mirror.getNodePath(),
                            sun_light_np,
                            reflection_receiver_np,
                            target_np,
                            mirror_camera_bitmask,
                        )
                        mirror_np = mirror.getNodePath()
                        mirror_np.setX((row - 1))
                        mirror_np.setZ((col - 1))
                        mirror_np.lookAt(0, FOCAL_LENGTH, 0)
                        self.mirrors.append(mirror)

        self.spot_mirrors = []
        if SPOT_SCREEN_ENABLED:
            # Notes :
            # - spot mirrors must be near the center of the main mirror
            # because multi_mirror_estimator works with a rigid set of mirror reflections
            # - all mirrors (including main mirror) must be aligned (all rays must be almost coplanar)
            # to the estimation algorithm to work correctly

            spot_mirror_x_offsets = [0, 0.02, 0.04]
            spot_mirror_heads = [-4, -6, -12]
            spot_mirror_pitchs = [2, 3, 6]

            for x_offset, head, pitch in zip(
                spot_mirror_x_offsets, spot_mirror_heads, spot_mirror_pitchs
            ):
                spot_mirror = SolarMirror(
                    self.orientable_frame_np,
                    sun_light_np,
                    reflection_receiver_np,
                    target_np,
                    None,
                    create_fake_spotlight,
                )
                np = spot_mirror.getNodePath()
                np.setPos(x_offset, 0.02, 0)
                np.setHpr(head, pitch, 0)
                np.setScale(0.01, 1, 0.01)
                np.setEffect(CompassEffect.make(
                    base.render, CompassEffect.P_scale))
                self.spot_mirrors.append(spot_mirror)

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
        return self.main_mirror

    def getSpotMirrors(self):
        return self.spot_mirrors

    def getMainMirrorToSpotMirrorAnglesInRadian(self):
        main_mirror_to_spot_mirror_angles_in_radian = []
        main_mirror_np = self.main_mirror.getNodePath()
        mirror_direction = LVector3(0, 1, 0)
        for spot_mirror in self.spot_mirrors:
            spot_mirror_np = spot_mirror.getNodePath()
            spot_mirror_direction_in_main_mirror = main_mirror_np.getRelativeVector(
                spot_mirror_np, mirror_direction
            )
            main_mirror_to_spot_mirror_angles_in_radian.append(
                mirror_direction.angleRad(spot_mirror_direction_in_main_mirror)
            )
        return main_mirror_to_spot_mirror_angles_in_radian

    def setStandHeight(self, height):
        self.stand_np.setSz(height)
        self.orientable_frame_np.setZ(height)

    def setMirrorWidth(self, width):
        self.orientable_frame_np.setSx(width)

    def setMirrorHeight(self, height):
        self.orientable_frame_np.setSz(height)

    def setRotationRadius(self, radius):
        self.orientable_frame_wires_np.setSy(radius)
        self.main_mirror.getNodePath().setY(radius)
        for spot_mirror in self.spot_mirrors:
            spot_mirror.getNodePath().setY(radius)

    def setOrientationOffset(self, head, pitch):
        self.mirror_head_offset = head
        self.mirror_pitch_offset = pitch

    def enableSunFollowing(self, sun_following_enabled):
        self.sun_following_enabled = sun_following_enabled

    def enableMainProjectionEstimationFromSpots(
        self, estimate_main_projection_from_spots
    ):
        self.estimate_main_projection_from_spots = estimate_main_projection_from_spots

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
        self.main_mirror.setDirectionLinesAlpha(alpha)
        for spot_mirror in self.spot_mirrors:
            spot_mirror.setDirectionLinesAlpha(alpha)
        for spot_mirror in self.mirrors:
            spot_mirror.setDirectionLinesAlpha(alpha)

    def sunFollowingTask(self, task):
        if self.settings.settingsHaveChanged():
            if (
                self.sun_following_enabled
                and self.settings.getMeasureValue(
                    "sun_direct_insolation_in_watt_per_square_meter"
                )
                > 0
            ):
                self.sun_follower.minimizeProjectionDistanceFromTarget(
                    self.estimate_main_projection_from_spots
                )
            else:
                self.orientable_frame_np.setHpr(
                    self.mirror_head + self.mirror_head_offset,
                    self.mirror_pitch + self.mirror_pitch_offset,
                    0,
                )

        return Task.cont
