from panda3d.core import *
from direct.showbase.ShowBase import ShowBase
from constants import *
from direct.task import Task

TEXTURE_WIDTH = 1024
CAMERA_FOCAL = 2

# A blank screen to measure spots on a different plane than measure_area
# which can happen in reality (we can project main ray further or closer)


class SpotScreen:
    def __init__(self, parent_np, settings):
        self.settings = settings

        # Create size-invariant root node
        # because external nodes can compute vector in this node path
        # (vector are silently modified by panda lib depending on node scale)
        # It's the bottom left corner of the screen
        self.screen_np = parent_np.attachNewNode("screen_origin")
        self.screen_np.setEffect(CompassEffect.make(
            base.render, CompassEffect.P_scale))

        self.measure_rectangle_np = self.screen_np.attachNewNode(
            CardMaker("measure_rectangle").generate()
        )
        texture = base.loader.loadTexture(MODELS_PATH + "concrete.jpg")
        texture.setWrapU(Texture.WMRepeat)
        texture.setWrapV(Texture.WMRepeat)
        self.measure_rectangle_np.setTexGen(
            TextureStage.getDefault(), TexGenAttrib.MWorldPosition
        )
        self.measure_rectangle_np.setTexTransform(
            TextureStage.getDefault(), TransformState.makeHpr((0, 90, 0))
        )
        self.measure_rectangle_np.setTexture(texture)
        self.measure_rectangle_np.setTwoSided(True)

        self.setupMeasuringCamera()

        screen_light = AmbientLight("screen_light")
        screen_light_np = render.attachNewNode(screen_light)
        self.measure_rectangle_np.setLight(screen_light_np)

        # Add a category in settings to contain child parameters created bellow
        settings.addTitle("Screen")

        settings.addSlider(
            "screen_x_in_meter",
            (0, 5),
            1,
            "X:",
            "m",
            self.screen_np.setX,
            1,
        )

        settings.addSlider(
            "screen_y_in_meter",
            (-5, 0),
            -0.5,
            "Y:",
            "m",
            self.screen_np.setY,
            1,
        )

        def updateWidth(width):
            height = self.measure_rectangle_np.getSz()
            self.measure_rectangle_np.setSx(width)
            self.measure_camera.setX(width / 2 - 0.2)
            self.measure_camera.node().getLens().setFilmSize(width + 0.5, height + 0.5)
            self.measure_camera.lookAt(
                self.screen_np, width / 2, 0, height / 2)

        settings.addSlider(
            "screen_width_in_meter",
            (0.1, 10),
            4,
            "Width:",
            "m",
            updateWidth,
            1,
        )

        def updateHeight(height):
            width = self.measure_rectangle_np.getSx()
            self.measure_rectangle_np.setSz(height)
            self.measure_camera.node().getLens().setFilmSize(width + 0.5, height + 0.5)
            self.measure_camera.lookAt(
                self.screen_np, width / 2, 0, height / 2)

        settings.addSlider(
            "screen_height_in_meter",
            (0.1, 5),
            3,
            "Height:",
            "m",
            updateHeight,
            1,
        )
        settings.addSlider(
            "screen_head_in_degree",
            (-60, 0),
            -10,
            "Head:",
            "°",
            self.screen_np.setH,
            1,
        )
        settings.addSlider(
            "screen_ambient_light",
            (0, 1),
            0.2,
            "Ambient light:",
            "",
            screen_light.setColor,
            1,
        )

        # sort=70 allow to analyze the measure camera buffer after display has been computed
        # and before request is sent to eventual remote client
        taskMgr.add(self.screenCaptureTask, "screenCapture", sort=70)

    def setupMeasuringCamera(self):

        # Setup buffer and camera to measure spots
        measure_lens = PerspectiveLens()
        measure_lens.setFocalLength(CAMERA_FOCAL)
        measure_lens.setNearFar(0.2, 2 * CAMERA_FOCAL)

        self.measure_buffer = base.win.makeTextureBuffer(
            "buffer", TEXTURE_WIDTH, TEXTURE_WIDTH
        )
        self.measure_camera = base.makeCamera(
            self.measure_buffer, clearColor=False)
        self.measure_camera.setPos(0, -CAMERA_FOCAL, 0.1)
        # WARNING : I don't know why but if camera is parented to measure_rectangle_np
        # spotlight spots are not visible in texture buffer
        # (but are still visible in application main window)
        self.measure_camera.reparentTo(self.screen_np)
        self.measure_camera.node().setLens(measure_lens)
        self.measure_camera.node().showFrustum()

    def screenCaptureTask(self, task):
        return
        # TODO : add SpotDetector class to process this image and give spot positions
        if self.settings.settingsHaveChanged():
            self.measure_image = PNMImage()
            if self.measure_buffer.getScreenshot(self.measure_image):
                self.measure_image.write("screen.png")

        return Task.cont

    def getNodePath(self):
        return self.screen_np
