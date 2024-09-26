# Copyright (C) 2024 Rémi Peuchot (https://remipch.github.io/)
# This code is distributed under GNU GPL v3 license

import sys
sys.path.insert(0, "sources")  # keep this line here for the next imports # nopep8

from remote_control_client import RemoteControlClient
from constants import SOURCES_PATH
import signal
import subprocess
import time
from pytest import approx


# Just to have time to see things moving in the application
# (not required, 0 is ok to run the test quickly)
TEST_DELAY_SECONDS = 0.5

# To run only this test :
# cd solar_concentrator/software/simulator
# pytest -v -k "test_application" --capture=no

# End to end test allow to test the wider possible area of this application :
# - lauched from command line ("python3 solar_concentrator_simulator.py")
# - dynamically changing parameters with remote_control facility (as done by a real user with settings_panel)
# - validating numerical estimation done by the application as a whole

# Because starting the application is the slowest step, we start it once at the beginning
# and reuse the same application instance across all tests
# Benefit :
# - shortest execution time
# - tests are presented as a single linear story easy to follow
# Drawbacks :
# - each test start from an application state that depends on previously executed tests instead of a clean, known state (however it's always possible to reset every settings before a given test)


def test_application_with_predefined_settings_give_expected_measure():
    # Lauch app in separate process
    process = subprocess.Popen(
        ["python3", SOURCES_PATH + "solar_concentrator_simulator.py"],
        stderr=subprocess.STDOUT,
    )

    # Wait remote server to be ready (when app is ready) and connect to it
    remote_control_client = RemoteControlClient()

    # Initial application state with default settings give known measures
    print("\n===== Initial application state =====", flush=True)
    reply = remote_control_client.request(
        {},
        [
            "sun_azimuth_in_degree",
            "sun_elevation_in_degree",
            "sun_direct_insolation_in_watt_per_square_meter",
            "measure_area_power_in_watt",
        ],
    )
    assert reply["sun_azimuth_in_degree"] == approx(164, abs=1)
    assert reply["sun_elevation_in_degree"] == approx(20, abs=1)
    assert reply["sun_direct_insolation_in_watt_per_square_meter"] == approx(
        655, abs=1)
    assert reply["measure_area_power_in_watt"] == approx(633, abs=2)
    time.sleep(TEST_DELAY_SECONDS)

    # Start with a simple minimal environment
    print("\n===== Sun at the vertical, no shadow =====", flush=True)
    reply = remote_control_client.request(
        {
            # Sun at the vertical (latitude 0, 21/03 at 12:00)
            "latitude_in_degree": 0,
            "date_in_ten_days_count": 8,
            "time_in_fifteen_minutes_count": 48,
            # Background : no obstacle, no shadow
            "building_height_in_meter": 0,
            "tree_height_in_meter": 0,
            "wall_height_in_meter": 1,
            # Measure area : 1 m², same height as the panel
            "measure_area_offset_z_in_meter": 1,
            "measure_area_width_in_meter": 1,
            "measure_area_height_in_meter": 1,
            # Panel : single panel, 1 m², centered, sun following
            "panel_grid_x_in_meter": 0,
            "panel_grid_y_in_meter": -5,
            "panel_grid_z_in_meter": 1,
            "panel_grid_row_count": 1,
            "panel_grid_column_count": 1,
            "panel_mirror_with_in_meter": 1,
            "panel_mirror_height_in_meter": 1,
            "panel_rotation_radius_in_meter": 0.1,
            "panel_head_offset_in_degree": 0,
            "panel_pitch_offset_in_degree": 0,
            "sun_following_enabled": True,
        },
        [
            "sun_azimuth_in_degree",
            "sun_elevation_in_degree",
            "sun_direct_insolation_in_watt_per_square_meter",
            "measure_area_power_in_watt",
        ],
    )
    # Sun orientation should be 90°,90° but is slightly different because :
    # - a few minutes are lost because sideral_time solar_time drift is ignored (see updateSunLight method comment)
    # - time is set with a resolution of 15 minutes
    # So we get (82°,88°) in these conditions
    # Result measure_area_power_in_watt is insolation * cos(45°)
    assert reply["sun_azimuth_in_degree"] == approx(82, abs=1)
    assert reply["sun_elevation_in_degree"] == approx(88, abs=1)
    assert reply["sun_direct_insolation_in_watt_per_square_meter"] == approx(
        947, abs=1)
    assert reply["measure_area_power_in_watt"] == approx(656, abs=2)
    time.sleep(TEST_DELAY_SECONDS)

    # Enlarging measure area should give the same power
    # with the same panel orientation (sun_following disabled)
    print("\n===== Enlarge measure area =====", flush=True)
    reply = remote_control_client.request(
        {
            # Measure area : 4 m², in front of the panel
            "measure_area_width_in_meter": 2,
            "measure_area_height_in_meter": 2,
            "sun_following_enabled": False,
        },
        [
            "measure_area_power_in_watt",
        ],
    )
    assert reply["measure_area_power_in_watt"] == approx(651, abs=2)
    time.sleep(TEST_DELAY_SECONDS)

    # Shifting the panel 1 m on the right should divide the power by two
    # (only half light reach the measure area)
    # with the same panel orientation (sun_following disabled)
    print("\n===== Shift panel right =====", flush=True)
    reply = remote_control_client.request(
        {
            "panel_grid_x_in_meter": 1,
        },
        [
            "measure_area_power_in_watt",
        ],
    )
    assert reply["measure_area_power_in_watt"] == approx(326, abs=2)
    time.sleep(TEST_DELAY_SECONDS)

    # Shifting the panel 1 m down should divide the power by two
    # (only quarter light reach the measure area)
    # with the same panel orientation (sun_following disabled)
    print("\n===== Shift panel down =====", flush=True)
    reply = remote_control_client.request(
        {
            "panel_grid_z_in_meter": 0.5,
        },
        [
            "measure_area_power_in_watt",
        ],
    )
    assert reply["measure_area_power_in_watt"] == approx(163, abs=2)
    time.sleep(TEST_DELAY_SECONDS)

    # Recenter and shrink the panel horizontaly should divide the initial power by two
    # (only half light reach the measure area)
    # with the same panel orientation (sun_following disabled)
    print("\n===== Recenter and shrink horizontally =====", flush=True)
    reply = remote_control_client.request(
        {
            "panel_grid_x_in_meter": 0,
            "panel_grid_z_in_meter": 1,
            "panel_mirror_with_in_meter": 0.5,
        },
        [
            "measure_area_power_in_watt",
        ],
    )
    assert reply["measure_area_power_in_watt"] == approx(326, abs=2)
    time.sleep(TEST_DELAY_SECONDS)

    # Change sun orientation
    print("\n===== Recenter and move sun =====", flush=True)
    reply = remote_control_client.request(
        {
            # Sun on the east (latitude 45, 21/06 at 10:00)
            "latitude_in_degree": 45,
            "date_in_ten_days_count": 17,
            "time_in_fifteen_minutes_count": 40,
            # Measure area : 4 m², in front of the panel
            "measure_area_width_in_meter": 2,
            "measure_area_height_in_meter": 2,
            "measure_area_offset_z_in_meter": 0.5,
            # Panel : single panel, 1 m², centered, sun following
            "panel_grid_x_in_meter": 0,
            "panel_grid_y_in_meter": -10,
            "panel_grid_z_in_meter": 1,
            "panel_grid_row_count": 1,
            "panel_grid_column_count": 1,
            "panel_mirror_with_in_meter": 1,
            "panel_mirror_height_in_meter": 1,
            "panel_rotation_radius_in_meter": 0.1,
            "panel_head_offset_in_degree": 0,
            "panel_pitch_offset_in_degree": 0,
            "sun_following_enabled": True,
        },
        [
            "sun_azimuth_in_degree",
            "sun_elevation_in_degree",
            "sun_direct_insolation_in_watt_per_square_meter",
            "measure_area_power_in_watt",
        ],
    )
    assert reply["sun_azimuth_in_degree"] == approx(120, abs=1)
    assert reply["sun_elevation_in_degree"] == approx(57, abs=1)
    assert reply["sun_direct_insolation_in_watt_per_square_meter"] == approx(
        906, abs=1)
    assert reply["measure_area_power_in_watt"] == approx(742, abs=2)
    time.sleep(TEST_DELAY_SECONDS)

    # Doubling panels horizontally should double the power
    print("\n===== Add panel horizontally =====", flush=True)
    reply = remote_control_client.request(
        {
            "panel_grid_column_count": 2,
        },
        [
            "measure_area_power_in_watt",
        ],
    )
    assert reply["measure_area_power_in_watt"] == approx(1470, abs=2)
    time.sleep(TEST_DELAY_SECONDS)

    # Doubling panels vertically should double the power
    print("\n===== Add panels vertically =====", flush=True)
    reply = remote_control_client.request(
        {
            "panel_grid_row_count": 2,
            "panel_grid_z_in_meter": 0.5,
        },
        [
            "measure_area_power_in_watt",
        ],
    )
    assert reply["measure_area_power_in_watt"] == approx(2902, abs=2)
    time.sleep(TEST_DELAY_SECONDS)

    # Decrease sun elevation
    print("\n===== Decrease sun elevation =====", flush=True)
    reply = remote_control_client.request(
        {
            # Decrease sun elevation (21/01/2023)
            "date_in_ten_days_count": 2,
        },
        [
            "sun_azimuth_in_degree",
            "sun_elevation_in_degree",
            "sun_direct_insolation_in_watt_per_square_meter",
            "measure_area_power_in_watt",
        ],
    )
    assert reply["sun_azimuth_in_degree"] == approx(147, abs=1)
    assert reply["sun_elevation_in_degree"] == approx(19, abs=1)
    assert reply["sun_direct_insolation_in_watt_per_square_meter"] == approx(
        624, abs=1)
    assert reply["measure_area_power_in_watt"] == approx(2388, abs=2)
    time.sleep(TEST_DELAY_SECONDS)

    # Hiding sun on one panel should decrease power of one quarter
    print("\n===== Add shadow on one panel =====", flush=True)
    reply = remote_control_client.request(
        {
            "building_x_in_meter": -7.6,
            "building_y_in_meter": 10,
            "building_with_in_meter": 10,
            "building_height_in_meter": 10,
            "panel_grid_space_x_in_meter": 0.6,
        },
        [
            "measure_area_power_in_watt",
        ],
    )
    assert reply["measure_area_power_in_watt"] == approx(1780, abs=2)
    time.sleep(TEST_DELAY_SECONDS)

    # Hiding sun on 1.5 panel should give 5/8 of the initial power
    print("\n===== Add shadow on one and a half panel =====", flush=True)
    reply = remote_control_client.request(
        {
            "building_x_in_meter": -8.5,
        },
        [
            "measure_area_power_in_watt",
        ],
    )
    assert reply["measure_area_power_in_watt"] == approx(1481, abs=2)
    time.sleep(TEST_DELAY_SECONDS)

    # Hiding sun on 2 panels should give half of the initial power
    print("\n===== Add shadow on two panels vertically =====", flush=True)
    reply = remote_control_client.request(
        {
            "building_x_in_meter": -9.2,
            "building_y_in_meter": 11.4,
            "building_with_in_meter": 9,
            "building_height_in_meter": 12,
        },
        [
            "measure_area_power_in_watt",
        ],
    )
    assert reply["measure_area_power_in_watt"] == approx(1184, abs=2)
    time.sleep(TEST_DELAY_SECONDS)

    # Adding tree shadow should decrease power
    print("\n===== Add tree shadow on two panels =====", flush=True)
    reply = remote_control_client.request(
        {
            "tree_x_in_meter": -10,
            "tree_height_in_meter": 8.5,
        },
        [
            "measure_area_power_in_watt",
        ],
    )
    assert reply["measure_area_power_in_watt"] == approx(810, abs=2)
    time.sleep(TEST_DELAY_SECONDS)

    # Nicely ask the separate application process to interrupt and wait for it
    # It only happens if all previous asserts passed,
    # otherwise the application window is kept open to investigate the potential problems
    process.send_signal(signal.SIGINT)
    process.wait()
