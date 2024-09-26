# Copyright (C) 2024 Rémi Peuchot (https://remipch.github.io/)
# This code is distributed under GNU GPL v3 license

from panda3d.core import *
from direct.showbase.ShowBase import ShowBase
from constants import *

from sun_position import *


class Sun:
    def __init__(self, world_np, settings):
        self.settings = settings

        # Create root node path with a common light
        sun_light = DirectionalLight("sun_light")
        sun_light.setColor(SUN_LIGHT_COLOR)
        sun_light.setShadowCaster(True)
        sun_light.getLens().setNearFar(-30, 30)  # TODO compute from world size
        sun_light.getLens().setFilmSize(20, 20)  # TODO compute from world size
        sun_light.setCameraMask(SUN_LIGHT_BITMASK)
        sun_light.showFrustum()
        self.sun_light_np = world_np.attachNewNode(sun_light)
        world_np.setLight(self.sun_light_np)

        self.frustum_np = self.sun_light_np.find("frustum")
        self.frustum_np.setColor(1, 1, 0, 1)
        self.frustum_np.setTransparency(TransparencyAttrib.MAlpha)
        frustom_light = AmbientLight("sun_frustum_light")
        frustom_light.setColor((1, 1, 1, 1))
        self.frustum_np.setLight(render.attachNewNode(frustom_light))

        # Parameters are updated and used in 'update'
        self.latitude = 0
        self.month = 0
        self.day = 0
        self.hour = 0
        self.minute = 0

        settings.addTitle("Sun")

        def setLatitude(slider_value):
            self.latitude = slider_value
            self.updateSunLight()

        settings.addSlider(
            "latitude_in_degree",
            (0, 90),
            45,
            "Latitude:",
            "°",
            setLatitude,
        )

        def setDate(slider_value):
            # Allow to choose a date in the form of day/month
            # with day possible values = (01,11,21)
            # from slider_value in range (0,35)
            self.month = 1 + (slider_value // 3)
            self.day = 1 + 10 * (slider_value % 3)
            self.updateSunLight()
            return f"{self.day:02d}/{self.month:02d}"

        settings.addSlider(
            "date_in_ten_days_count",
            (0, 35),
            0,
            "Date:",
            "",
            setDate,
        )

        def setTime(slider_value):
            # Allow to choose a time in the form of hh:mm
            # with mm multiple of 15
            # from slider_value in range (0,95)
            self.hour = slider_value // 4
            self.minute = 15 * (slider_value % 4)
            self.updateSunLight()
            return f"{self.hour:02d}:{self.minute:02d}"

        settings.addSlider(
            "time_in_fifteen_minutes_count",
            (0, 95),
            44,  # 11:00
            "Time:",
            "",
            setTime,
        )

        def setFrustomAlpha(alpha):
            self.frustum_np.setAlphaScale(alpha)
            # format alpha as a percentage (for display purpose only)
            return f"{100*alpha:.0f}"

        settings.addSlider(
            "sun_frustum_visibility",
            (0, 1),
            0.7,
            "Frustum visibility:",
            "°",
            setFrustomAlpha,
            1,
        )

        settings.addMeasure(
            "sun_azimuth_in_degree", lambda value: f"Azimuth: {value} °"
        )

        settings.addMeasure(
            "sun_elevation_in_degree", lambda value: f"Elevation: {value} °"
        )

        settings.addMeasure(
            "sun_direct_insolation_in_watt_per_square_meter",
            lambda value: f"Direct insolation: {value:.0f} W/m2",
        )

    def getLightNodePath(self):
        return self.sun_light_np

    def updateSunLight(self):
        # Compute sun orientation at longitude=0 and timezone=0
        # It's approximatively the local solar time at any other
        # longitudes or timezone (within a few minutes due to sideral_time solar_time drift)
        # It allows to keep the application free from arbitrary offsets related to :
        # - arbitrary time_zone definition
        # - arbitrary local daylight saving politics
        # - local longitude
        # Which is acceptable for this application goals
        # In the same way, fixing the year to 2023 allow simple date/time interface
        # and provide good enough year progression simulation
        time = (2023, self.month, self.day, self.hour, self.minute, 0, 0)
        location = (self.latitude, 0)
        azimuth, elevation = computeSunPosition(time, location, True)
        direct_insolation = computeDirectInsolation(elevation)
        self.settings.setMeasureValue("sun_azimuth_in_degree", azimuth)
        self.settings.setMeasureValue("sun_elevation_in_degree", elevation)
        self.settings.setMeasureValue(
            "sun_direct_insolation_in_watt_per_square_meter", direct_insolation
        )

        self.sun_light_np.setHpr(-azimuth, -elevation, 0)
        # print(f'azimuth = {azimuth} ; elevation = {elevation}',flush=True)
        if elevation > 0:
            self.sun_light_np.node().setColor(SUN_LIGHT_COLOR)
            self.frustum_np.setColor(1, 1, 0, 1)
        else:
            self.sun_light_np.node().setColor((0, 0, 0, 1))
            self.frustum_np.setColor(0, 0, 0, 1)
