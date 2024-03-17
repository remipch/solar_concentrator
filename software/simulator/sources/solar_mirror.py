from panda3d.core import *
import sys
from direct.showbase.ShowBase import ShowBase
from direct.task import Task
from constants import *
from direct.filter.FilterManager import FilterManager

TEXTURE_WIDTH = 512

# An undocumented constraint (hardware-related ?) make these problems appear
# if we create more than 14 projectors :
# - some arbitrary projectors won't work (they don't project anything)
# - all shadows computed by 'sun_light.setShadowCaster(True)' are removed
# - error message :display:gsg:glgsg(error): Could not load program: created-shader (The program could not load.)
FIRST_MIRROR_CAMERA_BIT = FIRST_MIRROR_CAMERA_BITMASK.getHighestOnBit()
MAX_MIRROR_CAMERA_COUNT = 14
MAX_MIRROR_CAMERA_BIT = FIRST_MIRROR_CAMERA_BIT + MAX_MIRROR_CAMERA_COUNT - 1

# WARNING : don't create more than 14 projectors (see comment in panel_grid.py)


class SolarMirror:
    # 'mirror_camera_bitmask' :
    # - is used by measure_camera to see only the measure surface
    # - must be a unique bitmask across the whole app
    def __init__(
        self,
        parent_np,
        sun_light_np,
        reflection_receiver_np,
        target_np,
        mirror_camera_bitmask,
    ):
        self.sun_light_np = sun_light_np
        self.reflection_receiver_np = reflection_receiver_np
        self.target_np = target_np
        self.mirror_camera_bitmask = mirror_camera_bitmask
        self.sun_light_color = self.sun_light_np.node().getColor()

        # Create root node path
        # the reflective surface will be centered on mirror_np
        self.mirror_np = parent_np.attachNewNode("solar_mirror")

        metal_material = Material()
        metal_material.setShininess(20.0)
        metal_material.setAmbient((0.1, 0.1, 0.1, 1))
        metal_material.setDiffuse((0.1, 0.1, 0.1, 1))
        metal_material.setSpecular((5, 5, 5, 1))

        shiny_surface = self.mirror_np.attachNewNode(
            CardMaker("shiny_surface").generate()
        )
        shiny_surface.setPosHpr(-0.5, 0, 0.5, 0, 180, 0)
        shiny_surface.setMaterial(metal_material, 100)

        back_surface = self.mirror_np.attachNewNode(
            CardMaker("back_surface").generate()
        )
        back_surface.setPosHpr(-0.5, -0.01, -0.5, 0, 0, 0)
        back_surface.setColor(0.3, 0.3, 0.6, 1)

        # prevent self-shadowing artifacts
        # (https://discourse.panda3d.org/t/rendering-and-shading-issues-on-os-x/13225/10)
        back_surface.setDepthOffset(-2)

        # Create size-invariant center node to compute direction_lines and lens related objects
        # (vector are silently modified by panda lib depending on node scale)
        # Prevent camera and projector to be scaled with parent node because :
        # - it makes the lens 'film' not perpendicular to the direction of view
        #   (lens is not orthonormal if Sx!=Sy or Sx!=Sz or Sy!=Sz)
        # - so it makes the trapeze view of square measure_surface to be always measured
        #   as square anyway
        # - which will be always projected as square instead of trapeze
        self.center_np = self.mirror_np.attachNewNode("center")
        self.center_np.setEffect(CompassEffect.make(
            base.render, CompassEffect.P_scale))

        self.setupDirectionLines()

        # (see top comment of this file)
        assert self.mirror_camera_bitmask.getHighestOnBit() <= MAX_MIRROR_CAMERA_BIT
        self.setupSolarReflection()

        # sort=45 allows to update reflection orientation
        # before igloop system Task which draws the scene
        taskMgr.add(self.mirrorReflectionUpdateTask,
                    "mirrorReflectionUpdate", sort=45)

    def setupDirectionLines(self):

        lines_light = AmbientLight("mirror_lines_light")
        lines_light.setColor((1, 1, 1, 1))
        lines_light_np = render.attachNewNode(lines_light)
        self.center_np.setLight(lines_light_np)

        lines = LineSegs("mirror_direction")
        lines.setColor(0, 0, 1, 1)
        lines.moveTo(0, 0, 0)
        lines.drawTo(0, 100, 0)
        lines.setThickness(1)
        self.mirror_direction_line_np = self.center_np.attachNewNode(
            lines.create())
        self.mirror_direction_line_np.setTransparency(
            TransparencyAttrib.MAlpha)

        lines = LineSegs("sun_direction")
        lines.setColor(1, 0.3, 0, 1)
        lines.moveTo(0, 0, 0)
        lines.drawTo(0, 100, 0)
        lines.setThickness(1)
        self.sun_direction_line_np = self.center_np.attachNewNode(
            lines.create())
        self.sun_direction_line_np.setTransparency(TransparencyAttrib.MAlpha)

        self.reflection_direction_np = self.center_np.attachNewNode(
            "reflection_direction"
        )
        lines = LineSegs("reflection_direction")
        lines.setColor(1, 0, 0, 1)
        lines.moveTo(0, 0, 0)
        lines.drawTo(0, 100, 0)
        lines.setThickness(1)
        self.reflection_direction_line_np = self.reflection_direction_np.attachNewNode(
            lines.create()
        )
        self.reflection_direction_line_np.setTransparency(
            TransparencyAttrib.MAlpha)

        # Prevent the reflection_direction_line to project sun shadow
        # (https://discourse.panda3d.org/t/unwanted-nodes-are-casting-shadows-even-after-subnode-setautoshader/26052/2)
        self.mirror_direction_line_np.hide(SUN_LIGHT_BITMASK)
        self.sun_direction_line_np.hide(SUN_LIGHT_BITMASK)
        self.reflection_direction_line_np.hide(SUN_LIGHT_BITMASK)

    def setupSolarReflection(self):
        # Define flat material to measure light energy received and reflected by the mirror
        # - ambient-only material does not allow shadows
        # - diffuse parameter allow shadows to be computed by DirectionalLight
        measure_material = Material()
        measure_material.setAmbient((0, 0, 0, 0))
        measure_material.setDiffuse((1, 1, 1, 1))
        measure_material.setShininess(0.0)
        measure_material.setSpecular((0, 0, 0, 0))

        # Define a rectangle measure area where sun light will be measured
        measure_surface = self.mirror_np.attachNewNode(
            CardMaker("measure_surface").generate()
        )
        measure_surface.setPosHpr(-0.5, 0, 0.5, 0, 180, 0)

        # Ensure the measure surface is only enlighted by the sun
        # (the measure must be proportional to sun only)
        measure_surface.clearLight()
        measure_surface.setLight(self.sun_light_np)
        measure_surface.setMaterial(measure_material)

        # Hide measure surface from main camera
        measure_surface.hide(MAIN_CAMERA_BITMASK)

        # Common lens used by the camera to measure the reflection
        # and by the projector to reproject the same image
        lens = OrthographicLens()
        lens.setFilmSize(2, 2)  # TODO use mirror width and height

        # Create a camera to measure sun reflection on measure_surface
        props = FrameBufferProperties()
        # Disable alpha (otherwise panda slightly decrease image levels)
        props.setRgbaBits(8, 8, 8, 0)
        props.setDepthBits(0)
        self.texture_buffer = base.win.makeTextureBuffer(
            "buffer", TEXTURE_WIDTH, TEXTURE_WIDTH, fbp=props
        )
        self.texture_buffer.setClearColor((0, 0, 0, 1))  # back will be black
        measure_camera_np = base.makeCamera(
            self.texture_buffer, clearColor=False)
        measure_camera_np.reparentTo(self.reflection_direction_np)
        measure_camera_np.setPosHpr(0, 2, 0, 180, 0, 0)
        measure_camera_np.node().setLens(lens)
        measure_camera_np.node().setCameraMask(self.mirror_camera_bitmask)
        base.render.hide(self.mirror_camera_bitmask)
        measure_surface.showThrough(self.mirror_camera_bitmask)
        # measure_camera_np.node().showFrustum()

        self.measure_texture = self.texture_buffer.getTexture()
        # self.measure_texture = loader.loadTexture('maps/envir-reeds.png')
        self.measure_texture.setWrapU(SamplerState.WMBorderColor)
        self.measure_texture.setWrapV(SamplerState.WMBorderColor)
        self.measure_texture.setBorderColor((0, 0, 0, 1))

        # measure_material does not work exactly as expected :
        # it's a "diffuse only" material but still makes the reflected light depend
        # on the relative orientation between surface and sun light
        # So we create a custom filter to :
        # - threshold the color on measure_surface (separate light from shadow)
        # - give a flat color corresponding to reflected light
        # - modulate this flat color in function of
        #   angle between target plane normal and reflection ray vector
        manager = FilterManager(self.texture_buffer, measure_camera_np)
        tex = Texture()
        self.quad = manager.renderSceneInto(colortex=tex)
        self.quad.setShader(Shader.load(SOURCES_PATH + "threshold.sha"))
        self.quad.setShaderInput("tex", tex)
        self.quad.setShaderInput("threshold", 0.1)
        self.quad.setShaderInput(
            "full_color", (SUN_LIGHT_BLUE_LEVEL, SUN_LIGHT_BLUE_LEVEL, SUN_LIGHT_BLUE_LEVEL, 1))
        self.quad.setShaderInput("level", 1)

        # Create a projector to reproject the sun reflection
        # reflection_direction_np will be updated when the mirror/sun relative position changes
        # Note : the projector is created at the same place as camera because :
        # - projector also project backward
        # - it doesn't require to manually flip the texture if it's on the same side
        # - it's easier to associate both and recompute only reflection_direction_np position
        #   in update_reflection_task
        self.sun_reflection_projector_np = self.reflection_direction_np.attachNewNode(
            LensNode("sun_reflection_projector")
        )
        self.sun_reflection_projector_np.setPosHpr(0, 2, 0, 180, 0, 0)
        self.sun_reflection_projector_np.node().setLens(lens)
        # self.sun_reflection_projector_np.node().showFrustum()

        self.ts = TextureStage("ts")
        self.ts.setMode(TextureStage.MAdd)
        self.reflection_receiver_np.projectTexture(
            self.ts, self.measure_texture, self.sun_reflection_projector_np
        )

    def getNodePath(self):
        return self.mirror_np

    def updateMirrorReflection(self):
        sun_direction_in_sun = LVector3f(0, 1, 0)
        sun_direction_in_mirror = self.center_np.getRelativeVector(
            self.sun_light_np, sun_direction_in_sun
        )
        self.sun_direction_line_np.lookAt(
            -sun_direction_in_mirror.getX(),
            -sun_direction_in_mirror.getY(),
            -sun_direction_in_mirror.getZ(),
        )
        # print(f"sun_direction_in_mirror = {sun_direction_in_mirror}", flush=True)
        self.reflection_direction_np.lookAt(
            sun_direction_in_mirror.getX(),
            -sun_direction_in_mirror.getY(),
            sun_direction_in_mirror.getZ(),
        )

        # Modulate light level received by target plane in function of
        # angle between target plane normal and reflection ray vector
        target_normal_in_target = LVector3f(0, 1, 0)

        reflection_direction_in_target = self.target_np.getRelativeVector(
            self.reflection_direction_np, LVector3f(0, 1, 0)
        )
        reflection_direction_in_target.normalize()

        level = reflection_direction_in_target.dot(target_normal_in_target)

        self.quad.setShaderInput("level", level)

    def mirrorReflectionUpdateTask(self, task):
        if not self.mirror_np.isHidden():
            self.updateMirrorReflection()
            # image = PNMImage()
            # if self.texture_buffer.getScreenshot(image):
            # image.write("reflection.png")

        return Task.cont

    # Get the reflection ray of the mirror center
    # The ray is returned as a pair of point
    # - first point is the mirror center
    # - second point is a direction point toward the reflection
    # - they are expressed relatively to the given 'relative_np' node_path
    # Method updateMirrorReflection() must be called before this one
    # to obtain up-to-date reflection ray
    def getReflectionRay(self, relative_np):
        mirror_center_in_relative_np = relative_np.getRelativePoint(
            self.reflection_direction_np, LPoint3(0, 0, 0)
        )
        direction_point_in_relative_np = relative_np.getRelativePoint(
            self.reflection_direction_np, LPoint3(0, 1, 0)
        )
        return mirror_center_in_relative_np, direction_point_in_relative_np

    def setDirectionLinesAlpha(self, alpha):
        self.mirror_direction_line_np.setAlphaScale(alpha)
        self.sun_direction_line_np.setAlphaScale(alpha)
        self.reflection_direction_line_np.setAlphaScale(alpha)
