import math

# Given :
# - a main mirror
# - 3 additional spot mirrors (fixed to main mirror with a known angle)
# - the 3 spot mirror solar reflections in screen plane
# This class estimates :
# - the position of the main mirror reflection ray in screen plane
# - and this ray intersection with target XZ plane

# Note this class is independant from pand3d or other heavy library
# and can be used in production code
# TODO : move this class in a separate 'common_tools' folder (with 'sun_position')

# Math shortcuts
rad, deg = math.radians, math.degrees
cos = math.cos
sin = math.sin
tan = math.tan
atan = math.atan
atan2 = math.atan2


def distance(a_x, a_y, b_x, b_y):
    return math.sqrt((a_x - b_x) ** 2 + (a_y - b_y) ** 2)


class MultiMirrorEstimator:
    def __init__(self, main_mirror_to_spot_mirror_angles_in_radian):
        # Compute individual angles between rays
        # Note : ray angle is twice the mirror angle
        self.main_ray_to_spot0_ray_angle = (
            2 * main_mirror_to_spot_mirror_angles_in_radian[0]
        )
        self.spot0_ray_to_spot1_ray_angle = 2 * (
            main_mirror_to_spot_mirror_angles_in_radian[1]
            - main_mirror_to_spot_mirror_angles_in_radian[0]
        )
        self.spot1_ray_to_spot2_ray_angle = 2 * (
            main_mirror_to_spot_mirror_angles_in_radian[2]
            - main_mirror_to_spot_mirror_angles_in_radian[1]
        )

    def estimateMainProjectionInScreen(self, spot_projections_in_screen):
        spot0_x = spot_projections_in_screen[0].x
        spot0_z = spot_projections_in_screen[0].z
        spot1_x = spot_projections_in_screen[1].x
        spot1_z = spot_projections_in_screen[1].z
        spot2_x = spot_projections_in_screen[2].x
        spot2_z = spot_projections_in_screen[2].z

        spot0_1 = distance(spot0_x, spot0_z, spot1_x, spot1_z)
        spot0_2 = distance(spot0_x, spot0_z, spot2_x, spot2_z)
        spot1_2 = distance(spot1_x, spot1_z, spot2_x, spot2_z)

        spot_vector_angle = atan2(spot2_z - spot0_z, spot2_x - spot0_x)
        # TODO : check alignment of 3 spots
        # TODO : check order of 3 spots because we work with unsigned distances

        # First : consider the spot aligned
        # Because the spots are considered aligned :
        # - the problem is invariant by rotation around the line of spots on screen
        # - we can express the problem in any plane passing by the line of spots
        # (we can rotate this working plane around this line and get the same result)
        # Consequently, everything below is expressed in this arbitrary "working plane"

        alpha0 = self.main_ray_to_spot0_ray_angle
        alpha1 = self.spot0_ray_to_spot1_ray_angle
        alpha2 = self.spot1_ray_to_spot2_ray_angle

        tan_alpha1 = tan(alpha1)
        tan_alpha12 = tan(alpha1 + alpha2)

        tan_theta = (spot0_2 / tan_alpha12 - spot0_1 / tan_alpha1) / spot1_2
        theta = atan(tan_theta)

        mirror_y = (
            spot0_1 * (tan_theta * tan_alpha1 - 1) / (tan_alpha1 * (1 + tan_theta**2))
        )
        main_proj_to_spot0 = mirror_y * (tan(theta - alpha0) - tan_theta)

        # First condider main_proj aligned with (spot0 spot2) line
        # by extrapolating this line proportionally
        main_proj_x = spot0_x - (spot2_x - spot0_x) * main_proj_to_spot0 / spot0_2
        main_proj_z = spot0_z - (spot2_z - spot0_z) * main_proj_to_spot0 / spot0_2

        # Second : apply correction to main proj because spots are not aligned
        # even if the mirror directions are perfectly aligned
        # (reflection ray angle is twice the mirror direction angle and the original sun ray has
        # a non nul angle from main mirror)
        # Simple linear method is applied to compensate the non-alignment :
        # - compute non alignment offset of spot1 with respect to (spot0 spot2) line
        # - extrapolate linarly this offset and apply on main_proj
        # This solution is fast and simple but adds a little error
        # (up to a few centimeters in bad conditions)
        # If more precision is required for this estimation, a minimization solution could be applied :
        # - from this initial state
        # - modelling the mirror reflections
        # - taking sun orientation into account
        # - minimizing the global difference between spot estimations and measures
        spot1_aligned_x = spot0_x + (spot2_x - spot0_x) * spot0_1 / spot0_2
        spot1_aligned_z = spot0_z + (spot2_z - spot0_z) * spot0_1 / spot0_2
        delta_x = spot1_x - spot1_aligned_x
        delta_z = spot1_z - spot1_aligned_z
        main_proj_spot0 = distance(main_proj_x, main_proj_z, spot0_x, spot0_z)
        main_proj_x = main_proj_x - 2 * delta_x * main_proj_spot0 / spot0_1
        main_proj_z = main_proj_z - 2 * delta_z * main_proj_spot0 / spot0_1

        # return estimation in screen
        return (main_proj_x, main_proj_z)
