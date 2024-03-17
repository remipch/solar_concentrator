import sys

sys.path.insert(0, "sources")

from sun_position import computeSunPosition

# To run all tests :
#   cd solar_concentrator/software/simulator
#   pytest -v --capture=no

# Test a few sun positions at various :
# - days in year
# - hours in day
# - latitudes (near and far from equator)
# - longitudes
# Asserted results have been computed manually here : https://gml.noaa.gov/grad/solcalc/
# and slightly adjusted (+/- 0.01 ° as an acceptable numerical error)
def test_known_positions():
    # (year, month, day, hour, minute, second, timezone),(latitude, longitude) == (azimuth, elevation)
    assert computeSunPosition(
        (2022, 7, 4, 11, 20, 0, -6), (40.602778, -104.741667), True
    ) == (121.38, 61.91)
    assert computeSunPosition((1997, 2, 8, 11, 35, 0, 3), (-80, 40), True) == (
        15.75,
        24.62,
    )
    assert computeSunPosition((2008, 4, 12, 8, 14, 0, 2), (-30, 30), True) == (
        63.91,
        23.19,
    )
    assert computeSunPosition((2012, 6, 15, 6, 53, 0, 7), (0, 100), True) == (
        66.47,
        7.58,
    )
    assert computeSunPosition((2023, 8, 7, 13, 21, 0, -8), (60, -160), True) == (
        151.33,
        43.69,
    )
