# Copyright (C) 2024 Rémi Peuchot
# This code is distributed under GNU GPL v3 license

from panda3d.core import *
from direct.showbase.ShowBase import ShowBase
from constants import *
from direct.task import Task

TEXTURE_WIDTH = 64


class MeasureArea:
    def __init__(self, parent_np, settings):
        self.settings = settings

        # Create size-invariant root node
        # because external nodes can compute vector in this node path
        # (vector are silently modified by panda lib depending on node scale)
        self.area_center_np = parent_np.attachNewNode("area_center")
        self.area_center_np.setEffect(
            CompassEffect.make(base.render, CompassEffect.P_scale)
        )

        self.resizable_area_np = self.area_center_np.attachNewNode(
            "resizable_area")

        self.measure_rectangle_np = self.resizable_area_np.attachNewNode(
            CardMaker("measure_rectangle").generate()
        )
        self.measure_rectangle_np.setPos(-0.5, 0.01, -0.5)
        # Just define pure black color without material
        # each mirror will add its own sun reflection to the image captured by measure_camera
        self.measure_rectangle_np.setColor(0, 0, 0, 1)

        self.setupMeasuringCamera()

        lines_light = AmbientLight("cross_lines_light")
        lines_light.setColor((1, 1, 1, 1))
        lines_light_np = render.attachNewNode(lines_light)

        # Add lines to display center (not viewed by measure camera)
        lines = LineSegs("cross")
        lines.setColor(0.8, 0.8, 0.8, 1)
        lines.moveTo(-0.1, 0, 0)
        lines.drawTo(0.1, 0, 0)
        lines.moveTo(0, 0, -0.1)
        lines.drawTo(0, 0, 0.1)
        lines.setThickness(1)
        self.cross_np = self.area_center_np.attachNewNode(lines.create())
        self.cross_np.setLight(lines_light_np)

        # Add a category in settings to contain child parameters created bellow
        settings.addTitle("Measure area")

        def updateOffsetZ(z_offset):
            # z_offset is the offset between parent_np and the bottom of resizable_area_np
            # offering a positive z_offset parameter instead of measure_area_z allow to garanty
            # that measure area is always above ground
            self.area_center_np.setZ(
                z_offset + self.resizable_area_np.getSz() / 2)

        settings.addSlider(
            "measure_area_offset_z_in_meter",
            (0, 5),
            0.5,
            "Offset z:",
            "m",
            updateOffsetZ,
            1,
        )
        settings.addSlider(
            "measure_area_width_in_meter",
            (0.1, 2),
            1,
            "Width:",
            "m",
            self.resizable_area_np.setSx,
            1,
        )

        def updateHeight(height):
            # z_offset is the offset between parent_np and the bottom of resizable_area_np
            # offering a positive z_offset parameter instead of measure_area_z allow to garanty
            # that measure area is always above ground
            z_offset = self.area_center_np.getZ() - self.resizable_area_np.getSz() / 2
            self.resizable_area_np.setSz(height)
            self.area_center_np.setZ(
                z_offset + self.resizable_area_np.getSz() / 2)

        settings.addSlider(
            "measure_area_height_in_meter",
            (0.1, 2),
            1,
            "Height:",
            "m",
            updateHeight,
            1,
        )
        settings.addMeasure(
            "measure_area_power_in_watt",
            lambda value: f"Estimated power: {value:0.0f} W",
        )

        # sort=65 allow to analyze the measure camera buffer after display has been computed
        # and before request is sent to eventual remote client
        taskMgr.add(self.powerMeasureTask, "powerMeasure", sort=65)

    def setupMeasuringCamera(self):

        # Setup buffer and camera to measure sun reflection
        measure_lens = OrthographicLens()
        measure_lens.setFilmSize(1, 1)
        measure_lens.setNearFar(0, 2)

        props = FrameBufferProperties()
        # Disable alpha (otherwise panda slightly decrease image levels)
        props.setRgbaBits(8, 8, 8, 0)
        props.setDepthBits(0)
        self.measure_buffer = base.win.makeTextureBuffer(
            "buffer", TEXTURE_WIDTH, TEXTURE_WIDTH, fbp=props
        )
        self.measure_buffer.setSort(0)
        measure_camera = base.makeCamera(self.measure_buffer, clearColor=False)
        measure_camera.reparentTo(self.measure_rectangle_np)
        measure_camera.setPos(0.5, -1, 0.5)
        measure_camera.node().setLens(measure_lens)
        measure_camera.node().setCameraMask(MEASURE_CAMERA_BITMASK)
        base.render.hide(MEASURE_CAMERA_BITMASK)
        self.measure_rectangle_np.showThrough(MEASURE_CAMERA_BITMASK)

    def powerMeasureTask(self, task):
        self.measure_image = PNMImage()
        if self.measure_buffer.getScreenshot(self.measure_image):
            # self.measure_image.write("measure.png")
            # The blue level is used to represent light power reflected by mirrors
            self.measure_image.makeGrayscale(0, 0, 1)

            # The average blue level of measure_image is proportional to
            # - sum of sun reflection received by the measure area
            # - blue level of the sun light which is projected from mirrors to measure area
            # - surface of the measure area

            # The power received by the measure area is proportional to :
            # - average blue level of measure_image
            # - inverse of blue level of the sun light
            # - direct solar insolation (previously computed in Sun class)
            # - surface of the measure area (which actually is the scale of the root node of self)
            measure_image_average_blue_level = self.measure_image.getAverageGray()
            measure_area_surface = (
                self.resizable_area_np.getSx() * self.resizable_area_np.getSz()
            )
            direct_insolation = self.settings.getMeasureValue(
                "sun_direct_insolation_in_watt_per_square_meter"
            )
            measure_area_power = (
                (measure_image_average_blue_level / SUN_LIGHT_BLUE_LEVEL)
                * direct_insolation
                * measure_area_surface
            )
            self.settings.setMeasureValue(
                "measure_area_power_in_watt", measure_area_power
            )

        return Task.cont

    def getNodePath(self):
        return self.area_center_np
