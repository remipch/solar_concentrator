from panda3d.core import *
import sys
from direct.showbase.ShowBase import ShowBase
from measure_area import MeasureArea
from constants import *
from spot_screen import SpotScreen


FIELD_WIDTH = 20
FIELD_DEPTH = 13
MAX_WALL_HEIGHT = 10


class Field:
    def __init__(self, parent_np, settings):

        # Create root node path with a common light
        self.field_np = parent_np.attachNewNode("field")
        field_light = AmbientLight("field_light")
        field_light.setColor((0.4, 0.4, 0.4, 1))
        self.field_np.setLight(render.attachNewNode(field_light))

        # Add ground with grass texture
        ground = self.field_np.attachNewNode(CardMaker("ground").generate())
        ground.setPosHpr(-FIELD_WIDTH / 2, -FIELD_DEPTH, 0, 0, -90, 0)
        ground.setSx(FIELD_WIDTH)
        ground.setSz(FIELD_DEPTH)
        grass_texture = base.loader.loadTexture(MODELS_PATH + "grass.jpg")
        grass_texture.setWrapU(Texture.WMRepeat)
        grass_texture.setWrapV(Texture.WMRepeat)
        ground.setTexGen(TextureStage.getDefault(), TexGenAttrib.MWorldPosition)
        ground.setTexScale(TextureStage.getDefault(), 0.5, 0.5)
        ground.setTexture(grass_texture)

        # Add wall with wood texture
        wall = self.field_np.attachNewNode(CardMaker("wall").generate())
        wall.setPos(-FIELD_WIDTH / 2, 0, 0)
        wall.setSx(FIELD_WIDTH)
        wood_texture_stage = TextureStage("wood_texture_stage")
        wood_texture = base.loader.loadTexture(MODELS_PATH + "wood.jpg")
        wood_texture.setWrapU(Texture.WMRepeat)
        wall.setTexture(wood_texture_stage, wood_texture)
        # Update the wood texture scale to repeat fixed-size pattern along the width
        wall.setTexScale(wood_texture_stage, FIELD_WIDTH / 3, 1)

        # Appear in last added title : "Background"
        settings.addSlider(
            "wall_height_in_meter", (1, 10), 4, "Wall height:", "m", wall.setSz
        )

        # Add measure area
        # (after field settings creation, so measure area settings appear after)
        self.measure_area = MeasureArea(self.field_np, settings)
        self.measure_area.getNodePath().setY(-0.02)  # just before wall

        if SPOT_SCREEN_ENABLED:
            self.screen = SpotScreen(self.field_np, settings)

    def getNodePath(self):
        return self.field_np

    def getTargetNodePath(self):
        return self.measure_area.getNodePath()
