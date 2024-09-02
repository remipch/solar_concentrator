# Copyright (C) 2024 Rémi Peuchot
# This code is distributed under GNU GPL v3 license

from panda3d.core import *
import sys
from direct.showbase.ShowBase import ShowBase
from constants import MODELS_PATH

TREE_MODEL_HEIGHT = 10
TREE_POS_Y = 5


class Background:
    def __init__(self, parent_np, settings):

        # Create root node path with a common light
        background_np = parent_np.attachNewNode("background")
        background_light = AmbientLight("background_light")
        background_light.setColor((0.4, 0.4, 0.4, 1))
        background_np.setLight(render.attachNewNode(background_light))

        # Add a category in settings to contain child parameters created bellow
        settings.addTitle("Background")

        # Create child nodes
        self.createBuilding(background_np, settings)
        self.createTree(background_np, settings)

    def createBuilding(self, background_np, settings):
        # Building origin is the center point of the low back line of the box
        building = background_np.attachNewNode("building")
        box = base.loader.loadModel(MODELS_PATH + "box")
        box.reparent_to(building)
        box.setPos(-0.5, 0, 0)

        settings.addSlider(
            "building_x_in_meter", (-20,
                                    20), 0, "Building x:", "m", building.setX, 1
        )
        settings.addSlider(
            "building_y_in_meter", (0,
                                    50), 10, "Building y:", "m", building.setY, 1
        )

        def updateBuildingWidth(width):
            building.setSx(width)
            building.setSy(width)

        settings.addSlider(
            "building_with_in_meter",
            (1, 50),
            10,
            "Building width:",
            "m",
            updateBuildingWidth,
        )

        def updateBuildingHeight(height):
            if height == 0:
                building.hide()
            else:
                building.show()
                building.setSz(height)

        settings.addSlider(
            "building_height_in_meter",
            (0, 50),
            6,
            "Building height:",
            "m",
            updateBuildingHeight,
        )

    def createTree(self, background_np, settings):
        tree = base.loader.loadModel(MODELS_PATH + "tree9/tree9.dae")
        tree.reparent_to(background_np)
        tree.setPos(0, TREE_POS_Y, 0)

        settings.addSlider(
            "tree_x_in_meter", (-20, 20), 5, "Tree x:", "m", tree.setX, 1
        )

        def updateTreeHeight(height):
            if height == 0:
                tree.hide()
            else:
                tree.show()
                tree.setScale(height / TREE_MODEL_HEIGHT)

        settings.addSlider(
            "tree_height_in_meter",
            (0, 20),
            9,
            "Tree height:",
            "m",
            updateTreeHeight,
            1,
        )
