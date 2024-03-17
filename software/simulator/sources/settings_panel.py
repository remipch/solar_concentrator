from panda3d.core import *
import sys
from direct.gui.DirectGui import *
from direct.task import Task

# Following dimensions and positions are expressed in 'a2dTopLeft' coordinate systems :
GLOBAL_SCALE = 0.75
LINE_INTERDISTANCE = 0.06
TITLE_TEXT_SCALE = 0.055
TITLE_TEXT_OFFSET_X = 0.01
TITLE_TEXT_OFFSET_Z = 0.01
TEXT_SCALE = 0.05
TEXT_OFFSET_X = 0.03
TEXT_OFFSET_Z = 0.02
SLIDER_MARGIN_X = 0.02
SLIDER_MARGIN_Z = 0.02
PANEL_WIDTH = 1.5
LABEL_WIDTH = 0.7

BACK_COLOR = (0.5, 0.4, 0.4, 1)
TITLE_COLOR = (0.05, 0.05, 0.05, 1)
TEXT_COLOR = (0.2, 0.2, 0.2, 1)
MEASURE_COLOR = (0.05, 0.4, 0.05, 1)
ENABLED_COLOR = (0.05, 0.4, 0.05, 1)
DISABLED_COLOR = (0.2, 0.2, 0.2, 1)


class SettingsPanel:
    """
    Base class to help to build the application settings panel.

    The settings panel consists of several lines.

    Each line can be either of :
    - a static title (for separating parameters by category)
    - a dynamic value :
        - input (set by user interface or remotely)
            - slider with a label (to change a specific parameter value)
            - checkbox (shown as a simple filled rectangle : green=enabled ; gray=disabled)
        - output ()
            - numerical measure value

    Each dynamic value has an identifier:
    - must be a string unique across the whole settings panel
    - defined at the creation time of the line
    - allowing to change/read the corresponding value programmatically

    To define the specific application settings panel you can :
    - either derive this class and call 'addTitle' and 'addSlider' methods in the constructor
    - or pass this class to various configurable objects and let them define their own parameters and the corresponding update behaviour

    As a first line, an initial slider is created to control the panel visibility.
    """

    def __init__(self):
        self.line_count = 1  # (visibility slider is the first line)
        self.sliders = []  # sliders are kept to pass custom argument to client callback
        # allow to retreive a control from it's identifier
        self.identifier_slider_dictionary = {}
        self.identifier_checkbox_dictionary = {}
        self.identifier_measure_dictionary = {}

        self.back_frame = DirectScrolledFrame(
            frameColor=BACK_COLOR,
            canvasSize=(0, PANEL_WIDTH, -2, 0),
            frameSize=(0, PANEL_WIDTH + 0.05, -2 / GLOBAL_SCALE, 0),
            manageScrollBars=True,
            autoHideScrollBars=True,
            scrollBarWidth=0.04,
            parent=base.a2dTopLeft,
        )
        self.back_frame.setTransparency(TransparencyAttrib.MAlpha)
        self.back_frame.setScale(GLOBAL_SCALE)

        def updateSettingsVisibility():
            alpha = round(settings_visibility_slider["value"], 1)
            settings_visibility_slider[
                "text"
            ] = f"Settings visibility: {100*alpha:.0f} %"
            self.back_frame.setAlphaScale(alpha)
            if alpha > 0.01:
                self.back_frame.show()
            else:
                self.back_frame.hide()

        settings_visibility_slider = DirectSlider(
            range=(0, 1),
            value=0.7,
            command=updateSettingsVisibility,
            text="placeholder text required for other text parameters to be taken into account",
            text_scale=TITLE_TEXT_SCALE,
            text_fg=TITLE_COLOR,
            text_pos=(
                TITLE_TEXT_OFFSET_X,
                -LINE_INTERDISTANCE + TITLE_TEXT_OFFSET_Z,
            ),
            scale=GLOBAL_SCALE,
            frameSize=(
                LABEL_WIDTH,
                PANEL_WIDTH - SLIDER_MARGIN_X,
                -LINE_INTERDISTANCE + SLIDER_MARGIN_Z,
                -SLIDER_MARGIN_Z,
            ),
            thumb_frameSize=(0.48, 0.52, -0.02, 0.02),
            text_align=TextNode.ALeft,
            parent=base.a2dTopLeft,
        )

        self.settings_changed = False
        taskMgr.add(
            self.resetSettingsChangedFlagTask, "resetSettingsChangedFlag", sort=-100
        )

        print("\nSettings created", flush=True)

    def resetSettingsChangedFlagTask(self, task):
        self.settings_changed = False
        return Task.cont

    # return True if at least one parameter has changed since the begining of the current frame
    def settingsHaveChanged(self):
        return self.settings_changed

    def assertUnkownIdentifier(self, identifier):
        assert identifier not in self.identifier_measure_dictionary
        assert identifier not in self.identifier_slider_dictionary
        assert identifier not in self.identifier_checkbox_dictionary

    def setSettingValue(self, identifier, value):
        if identifier in self.identifier_slider_dictionary:
            self.setSliderValue(identifier, value)
            return
        if identifier in self.identifier_checkbox_dictionary:
            self.setCheckboxValue(identifier, value)
            return
        assert False, "unknwon setting identifier"

    def addLine(self):
        self.line_count = self.line_count + 1
        self.back_frame["canvasSize"] = (
            0,
            PANEL_WIDTH,
            -(self.line_count + 0.5) * LINE_INTERDISTANCE,
            0,
        )

    def addTitle(self, title):
        text = DirectLabel(
            text=title,
            text_scale=TITLE_TEXT_SCALE,
            text_fg=TITLE_COLOR,
            text_pos=(
                TITLE_TEXT_OFFSET_X,
                -(self.line_count + 1) *
                LINE_INTERDISTANCE + TITLE_TEXT_OFFSET_Z,
            ),
            text_align=TextNode.ALeft,
            relief=None,
            parent=self.back_frame.getCanvas(),
        )
        self.addLine()
        print(f"  Title '{title}' added to settings", flush=True)

    # format_function get a numerical value and return the text to display
    def addMeasure(self, identifier, format_function):
        label = DirectLabel(
            text="placeholder text required for other text parameters to be taken into account",
            text_scale=TEXT_SCALE,
            text_fg=MEASURE_COLOR,
            text_pos=(
                TEXT_OFFSET_X,
                -(self.line_count + 1) * LINE_INTERDISTANCE + TEXT_OFFSET_Z,
            ),
            text_align=TextNode.ALeft,
            relief=None,
            parent=self.back_frame.getCanvas(),
        )
        self.assertUnkownIdentifier(identifier)
        self.identifier_measure_dictionary[identifier] = {
            "label": label,
            "format_function": format_function,
            "value": None,
        }
        self.addLine()
        print(f"    Measure '{identifier}' added to settings", flush=True)

    def setMeasureValue(self, identifier, value):
        measure = self.identifier_measure_dictionary[identifier]
        label = measure["label"]
        format_function = measure["format_function"]
        measure["value"] = value
        label["text"] = format_function(value)

    def getMeasureValue(self, identifier):
        measure = self.identifier_measure_dictionary[identifier]
        return measure["value"]

    # Add a slider at the end of the panel
    # 'callback' will be called :
    # - whenever the slider value changed
    # - with the new value as the first parameter
    # - with optional 'additional_parameter' if it is defined
    # If callback returns a non empty text, it will be shown as the new value
    # otherwise the slider value is float-formatted according to rounding_digit
    def addSlider(
        self,
        identifier,
        range,
        value,
        prefix,
        suffix,
        callback,
        rounding_digits=0,
        additional_parameter=None,
    ):
        def sliderCallback(slider_index, additional_parameter):
            slider = self.sliders[slider_index]
            slider_value = slider["value"]
            # in next line +0.0 allow to avoid displaying -0.0
            slider_value = round(slider_value, rounding_digits) + 0.0
            # eventually convert to int to avoid to displays misleading ".0"
            # (which contradicts rounding_digits value)
            if rounding_digits == 0:
                slider_value = int(slider_value)
            if additional_parameter is None:
                optional_text = callback(slider_value)
            else:
                optional_text = callback(slider_value, additional_parameter)
            if optional_text is None:
                display_text = str(slider_value)
            else:
                display_text = optional_text

            slider["text"] = prefix + " " + display_text + " " + suffix

            self.settings_changed = True

        slider = DirectSlider(
            range=range,
            value=value,
            command=sliderCallback,
            extraArgs=[len(self.sliders), additional_parameter],
            text_scale=TEXT_SCALE,
            text="placeholder text required for other text parameters to be taken into account",
            text_fg=TEXT_COLOR,
            frameSize=(
                LABEL_WIDTH,
                PANEL_WIDTH - SLIDER_MARGIN_X,
                -(self.line_count + 1) * LINE_INTERDISTANCE + SLIDER_MARGIN_Z,
                -self.line_count * LINE_INTERDISTANCE - SLIDER_MARGIN_Z,
            ),
            thumb_frameSize=(0.48, 0.52, -0.02, 0.02),
            text_pos=(
                TEXT_OFFSET_X,
                -(self.line_count + 1) * LINE_INTERDISTANCE + TEXT_OFFSET_Z,
            ),
            text_align=TextNode.ALeft,
            parent=self.back_frame.getCanvas(),
        )
        self.sliders.append(slider)
        self.assertUnkownIdentifier(identifier)
        self.identifier_slider_dictionary[identifier] = slider
        self.addLine()
        print(f"    Slider '{identifier}' added to settings", flush=True)

    def setSliderValue(self, identifier, value):
        slider = self.identifier_slider_dictionary[identifier]
        assert slider["range"][0] <= value <= slider["range"][1]
        slider.setValue(value)

    def checkboxCallback(self, identifier, mouse_event=None):
        checkbox_data = self.identifier_checkbox_dictionary[identifier]
        label = checkbox_data["label"]
        frame = checkbox_data["frame"]
        callback = checkbox_data["callback"]
        value = not checkbox_data["value"]
        checkbox_data["value"] = value
        prefix = checkbox_data["prefix"]
        additional_parameter = checkbox_data["additional_parameter"]
        if additional_parameter is None:
            optional_text = callback(value)
        else:
            optional_text = callback(value, additional_parameter)
        if optional_text is None:
            display_text = "enabled" if value else "disabled"
        else:
            display_text = optional_text
        label["text"] = prefix + " " + display_text
        frame["frameColor"] = ENABLED_COLOR if value else DISABLED_COLOR

        self.settings_changed = True

    def setCheckboxValue(self, identifier, value):
        checkbox_data = self.identifier_checkbox_dictionary[identifier]
        checkbox_data["value"] = not value
        self.checkboxCallback(identifier)

    def addCheckbox(
        self, identifier, value, prefix, callback, additional_parameter=None
    ):
        # After trying to use DirectCheckButton, it seems simpler to create
        # separate label and frame to control their own position and frame_size
        label = DirectLabel(
            text="placeholder text required for other text parameters to be taken into account",
            text_scale=TEXT_SCALE,
            text_fg=TEXT_COLOR,
            text_pos=(
                TEXT_OFFSET_X,
                -(self.line_count + 1) * LINE_INTERDISTANCE + TEXT_OFFSET_Z,
            ),
            text_align=TextNode.ALeft,
            relief=None,
            parent=self.back_frame.getCanvas(),
        )

        frame = DirectFrame(
            frameColor=(0.7, 0.7, 0.7, 1),
            frameSize=(
                LABEL_WIDTH,
                PANEL_WIDTH - SLIDER_MARGIN_X,
                -(self.line_count + 0.8) * LINE_INTERDISTANCE,
                -(self.line_count + 0.2) * LINE_INTERDISTANCE,
            ),
            parent=self.back_frame.getCanvas(),
        )
        # Make the frame clickable
        frame["state"] = DGG.NORMAL
        frame.bind(
            event=DGG.B1PRESS,
            command=self.checkboxCallback,
            extraArgs=[identifier],
        )

        checkbox_data = {
            "label": label,
            "frame": frame,
            "callback": callback,
            "value": not value,  # because callback will be called once to initialize it
            "prefix": prefix,
            "additional_parameter": additional_parameter,
        }
        self.assertUnkownIdentifier(identifier)
        self.identifier_checkbox_dictionary[identifier] = checkbox_data
        self.addLine()
        # Call callback once to initialize
        self.checkboxCallback(identifier)
        print(f"    Checkbox '{identifier}' added to settings", flush=True)
