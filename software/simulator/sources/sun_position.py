# It's a copy/paste from https://levelup.gitconnected.com/python-sun-position-for-solar-energy-and-research-7a4ead801777
# There is no specific license in this page but author says :
# "I’ve set this program up such that it’s easy to import into any program where you need to find the Sun’s position. Save the code in a file named sunpos.py, import it into your projects, and call the function sunpos() as needed."
# Thank you John Clark Craig for this code and the explanations

import math

# Math typing shortcuts
rad, deg = math.radians, math.degrees
sin, cos, tan = math.sin, math.cos, math.tan
asin, atan2 = math.asin, math.atan2


def computeSunPosition(time, location, refraction):
    # Extract the passed data
    year, month, day, hour, minute, second, timezone = time
    latitude, longitude = location

    # Convert latitude and longitude to radians
    rlat = rad(latitude)
    rlon = rad(longitude)

    # Decimal hour of the day at Greenwich
    greenwichtime = hour - timezone + minute / 60 + second / 3600

    # Days from J2000, accurate from 1901 to 2099
    daynum = (
        367 * year
        - 7 * (year + (month + 9) // 12) // 4
        + 275 * month // 9
        + day
        - 730531.5
        + greenwichtime / 24
    )

    # Mean longitude of the sun
    mean_long = daynum * 0.01720279239 + 4.894967873

    # Mean anomaly of the Sun
    mean_anom = daynum * 0.01720197034 + 6.240040768

    # Ecliptic longitude of the sun
    eclip_long = (
        mean_long
        + 0.03342305518 * sin(mean_anom)
        + 0.0003490658504 * sin(2 * mean_anom)
    )

    # Obliquity of the ecliptic
    obliquity = 0.4090877234 - 0.000000006981317008 * daynum

    # Right ascension of the sun
    rasc = atan2(cos(obliquity) * sin(eclip_long), cos(eclip_long))

    # Declination of the sun
    decl = asin(sin(obliquity) * sin(eclip_long))

    # Local sidereal time
    sidereal = 4.894961213 + 6.300388099 * daynum + rlon

    # Hour angle of the sun
    hour_ang = sidereal - rasc

    # Local elevation of the sun
    elevation = asin(sin(decl) * sin(rlat) + cos(decl)
                     * cos(rlat) * cos(hour_ang))

    # Local azimuth of the sun
    azimuth = atan2(
        -cos(decl) * cos(rlat) * sin(hour_ang),
        sin(decl) - sin(rlat) * sin(elevation),
    )

    # Convert azimuth and elevation to degrees
    azimuth = into_range(deg(azimuth), 0, 360)
    elevation = into_range(deg(elevation), -180, 180)

    # Refraction correction (optional)
    if refraction:
        targ = rad((elevation + (10.3 / (elevation + 5.11))))
        elevation += (1.02 / tan(targ)) / 60

    # Return azimuth and elevation in degrees
    return (round(azimuth, 2), round(elevation, 2))


def into_range(x, range_min, range_max):
    shiftedx = x - range_min
    delta = range_max - range_min
    return (((shiftedx % delta) + delta) % delta) + range_min


# Compute direct solar insolation in watt per square meter :
# - direct insolation (excluding diffuse insolation)
# - perpendicular to the sun's rays
# - for a clear sky without clouds
# - remove the estimated atmospheric losses due to absorption and scattering
def computeDirectInsolation(sun_elevation_in_degree):
    if sun_elevation_in_degree <= 0:
        # don't compute solar insolation if sun is suncined
        # (following computation would give complex number)
        return 0
    # Implement simplified formula from :
    # https://en.wikipedia.org/wiki/Direct_insolation#Simplified_formula
    sun_zenith_in_rad = rad(90 - sun_elevation_in_degree)
    airmass = 1 / cos(sun_zenith_in_rad)
    return 1353 * (0.7 ** (airmass**0.678))
