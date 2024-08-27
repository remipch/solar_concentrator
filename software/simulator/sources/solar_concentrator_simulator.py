from direct.showbase.ShowBase import ShowBase
from panda3d.core import *
import sys
from direct.gui.OnscreenText import OnscreenText
from direct.gui.DirectGui import *
from camera_control import CameraControl
from settings_panel import SettingsPanel
from direct.task import Task
from background import Background
from field import Field
from panel_grid import PanelGrid
from sun import Sun
from constants import *
from remote_control_server import RemoteControlServer

# Force opengl version.
# Later versions do not render texture properly (ignore texture attributes)
loadPrcFileData("", "gl-version 3 0")

arg_count = len(sys.argv)

remote_control_enabled = True
if arg_count > 1 and sys.argv[1] == "--disable-remote-control":
    remote_control_enabled = False


class MyApp(ShowBase):
    def __init__(self):
        ShowBase.__init__(self)

        camera_control = CameraControl()

        # This resolution makes the various texts displayed in render2d readable
        # (this overrides the resolution in Config.prc)
        props = WindowProperties()
        props.setTitle("Solar concentrator simulator")
        props.setSize(1000, 800)
        self.win.requestProperties(props)

        self.accept("escape", sys.exit)

        self.render.setShaderAuto()

        base.cam.node().setCameraMask(MAIN_CAMERA_BITMASK)

        settings = SettingsPanel()

        world_np = self.render.attachNewNode("world")
        sun = Sun(world_np, settings)
        background = Background(world_np, settings)
        field = Field(world_np, settings)
        panel_grid = PanelGrid(
            world_np,
            settings,
            sun.getLightNodePath(),
            field.getNodePath(),
            field.getTargetNodePath(),
        )
        if remote_control_enabled:
            remote_control_server = RemoteControlServer(settings)

        # self.render.ls()
        print(taskMgr, flush=True)
        self.accept("v", base.bufferViewer.toggleEnable)


app = MyApp()
app.run()
