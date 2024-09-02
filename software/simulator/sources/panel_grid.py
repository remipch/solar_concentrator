# Copyright (C) 2024 Rémi Peuchot
# This code is distributed under GNU GPL v3 license

from panda3d.core import *
import sys
from direct.showbase.ShowBase import ShowBase
from mirror_panel import MirrorPanel
from constants import *

if MULTI_MIRROR_ENABLED:
    MAX_ROW_COUNT = 1
    MAX_COLUMN_COUNT = 1
else:
    # (see comment in solar_mirror.py)
    # If more than 14 : problem with solar reflection
    MAX_ROW_COUNT = 2
    MAX_COLUMN_COUNT = 5


class PanelGrid:
    def __init__(
        self, parent_np, settings, sun_light_np, reflection_receiver_np, target_np
    ):

        # Create root node path with a common light
        self.panel_grid_np = parent_np.attachNewNode("panel_grid")
        panel_grid_light = AmbientLight("panel_grid_light")
        panel_grid_light.setColor((0.4, 0.4, 0.4, 1))
        self.panel_grid_np.setLight(render.attachNewNode(panel_grid_light))

        # Parameters are updated and used in 'update' methods to eventually recompute distribution globally
        self.grid_z = 0
        self.row_count = 1
        self.column_count = 1
        self.panel_space_x = 0
        self.panel_space_y = 0
        self.panel_space_z = 0
        self.panel_rotation_radius = 0.1
        self.mirror_width = 1
        self.mirror_height = 1
        self.head_offset = 0
        self.pitch_offset = 0
        self.sun_following_enabled = True
        self.parabolic_mirrors_enabled = True

        # Create every possible panels, they will be shown/hidden dynamically
        self.panels = []
        mirror_camera_bitmask = FIRST_MIRROR_CAMERA_BITMASK
        for row in range(MAX_ROW_COUNT):
            panel_row = []
            for col in range(MAX_COLUMN_COUNT):
                panel_row.append(
                    MirrorPanel(
                        self.panel_grid_np,
                        sun_light_np,
                        reflection_receiver_np,
                        target_np,
                        mirror_camera_bitmask,
                        settings,
                    )
                )
                mirror_camera_bitmask = mirror_camera_bitmask << 1
            self.panels.append(panel_row)

        # Add a category in settings to contain child parameters created bellow
        settings.addTitle("Panel grid")
        settings.addSlider(
            "panel_grid_x_in_meter",
            (-10, 10),
            0,
            "Grid x:",
            "m",
            self.panel_grid_np.setX,
        )
        settings.addSlider(
            "panel_grid_y_in_meter",
            (-20, -1),
            -10,
            "Grid y:",
            "m",
            self.panel_grid_np.setY,
        )
        settings.addSlider(
            "panel_grid_z_in_meter",
            (0, 4),
            1,
            "Grid z:",
            "m",
            self.updateParameterAndRecomputeGridLayout,
            1,
            "grid_z",
        )
        if not MULTI_MIRROR_ENABLED:
            settings.addSlider(
                "panel_grid_row_count",
                (1, MAX_ROW_COUNT),
                1,
                "Row count:",
                "",
                self.updateParameterAndRecomputeGridLayout,
                0,
                "row_count",
            )
            settings.addSlider(
                "panel_grid_column_count",
                (1, MAX_COLUMN_COUNT),
                1,
                "Column count:",
                "",
                self.updateParameterAndRecomputeGridLayout,
                0,
                "column_count",
            )
            settings.addSlider(
                "panel_grid_space_x_in_meter",
                (0, 1),
                0.5,
                "Panel space x:",
                "m",
                self.updateParameterAndRecomputeGridLayout,
                1,
                "panel_space_x",
            )
            settings.addSlider(
                "panel_grid_space_y_in_meter",
                (0, 1),
                0.5,
                "Panel space y:",
                "m",
                self.updateParameterAndRecomputeGridLayout,
                1,
                "panel_space_y",
            )
            settings.addSlider(
                "panel_grid_space_z_in_meter",
                (0, 1),
                0.5,
                "Panel space z:",
                "m",
                self.updateParameterAndRecomputeGridLayout,
                1,
                "panel_space_z",
            )

        settings.addTitle("Panels")

        settings.addSlider(
            "panel_mirror_with_in_meter",
            (0, 2),
            1,
            "Mirror width:",
            "m",
            self.updateParameterAndRecomputeGridLayout,
            1,
            "mirror_width",
        )

        settings.addSlider(
            "panel_mirror_height_in_meter",
            (0, 2),
            1,
            "Mirror height:",
            "m",
            self.updateParameterAndRecomputeGridLayout,
            1,
            "mirror_height",
        )

        settings.addSlider(
            "panel_rotation_radius_in_meter",
            (0.01, 0.5),
            0.1,
            "Rotation radius:",
            "m",
            self.updateParameter,
            2,
            "panel_rotation_radius",
        )

        settings.addSlider(
            "panel_head_offset_in_degree",
            (-20, 20),
            0,
            "Head offset:",
            "°",
            self.updateParameter,
            0,
            "head_offset",
        )

        settings.addSlider(
            "panel_pitch_offset_in_degree",
            (-20, 20),
            0,
            "Pitch offset:",
            "°",
            self.updateParameter,
            0,
            "pitch_offset",
        )

        settings.addCheckbox(
            "sun_following_enabled",
            True,
            "Sun following:",
            self.updateParameter,
            "sun_following_enabled",
        )

        if MULTI_MIRROR_ENABLED:
            settings.addCheckbox(
                "parabolic_mirrors_enabled",
                True,
                "Parabolic mirrors:",
                self.updateParameter,
                "parabolic_mirrors_enabled",
            )

        settings.addSlider(
            "panel_direction_lines_visibility_ratio",
            (0, 1),
            0,
            "Directions visibility:",
            "%",
            self.updateDirectionLinesVisibility,
            1,
        )

    def recomputeGridLayout(self):
        for row in range(MAX_ROW_COUNT):
            for col in range(MAX_COLUMN_COUNT):
                panel = self.panels[row][col]
                panel_np = panel.getNodePath()

                panel_np.setX(
                    (self.mirror_width + self.panel_space_x)
                    * (col - (self.column_count - 1) / 2)
                )
                panel_np.setY(-row * self.panel_space_y)

    def updateParameter(self, value, param_name):
        setattr(self, param_name, value)
        for row in range(MAX_ROW_COUNT):
            for col in range(MAX_COLUMN_COUNT):
                panel = self.panels[row][col]
                panel_np = panel.getNodePath()

                if row >= self.row_count or col >= self.column_count:
                    panel_np.hide()
                    continue

                panel_np.show()
                panel.setStandHeight(
                    self.grid_z
                    + self.mirror_height / 2
                    + row * (self.panel_space_z + self.mirror_height)
                )
                panel.setMirrorWidth(self.mirror_width)
                panel.setMirrorHeight(self.mirror_height)
                panel.setRotationRadius(self.panel_rotation_radius)
                panel.setOrientationOffset(self.head_offset, self.pitch_offset)
                panel.enableSunFollowing(self.sun_following_enabled)
                panel.enableParabolicMirrors(self.parabolic_mirrors_enabled)

    def updateParameterAndRecomputeGridLayout(self, value, param_name):
        self.updateParameter(value, param_name)
        self.recomputeGridLayout()

    def updateDirectionLinesVisibility(self, alpha):
        # Update all parameters
        for row in range(MAX_ROW_COUNT):
            for col in range(MAX_COLUMN_COUNT):
                panel = self.panels[row][col]
                panel.setDirectionLinesAlpha(alpha)
        # format alpha as a percentage (for display purpose only)
        return f"{100*alpha:.0f}"
