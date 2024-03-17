from panda3d.core import *
from direct.showbase.ShowBase import ShowBase
import math
import numpy as np
from scipy.optimize import minimize
from multi_mirror_estimator import MultiMirrorEstimator
from constants import *


def norm(point):
    return math.sqrt(point.x**2 + point.y**2 + point.z**2)


class SunFollower:
    def __init__(
        self,
        mirror_panel,  # to control mirror orientation
        target_np,  # to compute projection distance of reflection ray projection
    ):
        self.mirror_panel = mirror_panel
        self.main_mirror = mirror_panel.getMainMirror()
        self.spot_mirrors = mirror_panel.getSpotMirrors()
        self.target_np = target_np
        
        if SPOT_SCREEN_ENABLED:
            self.multi_mirror_estimator = MultiMirrorEstimator(
                mirror_panel.getMainMirrorToSpotMirrorAnglesInRadian()
            )

        # XZ target plane is :
        # - where we project mirror center reflection ray
        # - and compute the error to minimize (distance between target and projection)
        self.target_xz_plane = Plane(Vec3(0, 1, 0), Point3(0, 0, 0))

    def computeRayIntersectionWithTargetXZPlane(self, mirror):
        mirror.updateMirrorReflection()

        # Project the mirror reflection ray to target XZ plane
        (
            mirror_center_in_relative_np,
            direction_point_in_relative_np,
        ) = mirror.getReflectionRay(self.target_np)

        reflection_projection_in_target = LPoint3()

        if not self.target_xz_plane.intersectsLine(
            reflection_projection_in_target,
            mirror_center_in_relative_np,
            direction_point_in_relative_np,
        ):
            return None

        if norm(mirror_center_in_relative_np - reflection_projection_in_target) > norm(
            direction_point_in_relative_np - reflection_projection_in_target
        ):
            # mirror center is farest than direction point
            # indicates that mirror is directed toward the target
            return reflection_projection_in_target

        # Otherwise the intersection is not on the target plane
        return None

    # Compute distance between :
    # - main mirror ray center intersection with target XZ plane
    # - and target center
    def computeMainProjectionError(self, mirror_head_pitch):
        mirror_head = mirror_head_pitch[0]
        mirror_pitch = mirror_head_pitch[1]

        # Orient the mirror with the given head and pitch
        self.mirror_panel.setMirrorOrientationWithoutOffset(mirror_head, mirror_pitch)

        reflection_projection_in_target = self.computeRayIntersectionWithTargetXZPlane(
            self.main_mirror
        )
        if reflection_projection_in_target is None:
            return 10000
        return norm(reflection_projection_in_target)

    # Estimate distance between :
    # - main mirror ray center intersection with target XZ plane
    # - and target center
    # Using only the spot mirror intersections with target XZ plane
    def estimateMainProjectionErrorFromSpots(self, mirror_head_pitch):
        mirror_head = mirror_head_pitch[0]
        mirror_pitch = mirror_head_pitch[1]

        # Orient the mirror with the given head and pitch
        self.mirror_panel.setMirrorOrientationWithoutOffset(mirror_head, mirror_pitch)

        spot_projections_in_screen = []
        for spot_mirror in self.spot_mirrors:
            spot_projection_in_screen = self.computeRayIntersectionWithTargetXZPlane(
                spot_mirror
            )
            if spot_projection_in_screen is None:
                print("cannot compute spot projection", flush=True)
                return 10000

            spot_projections_in_screen.append(spot_projection_in_screen)

        main_projection_estimation_in_screen = (
            self.multi_mirror_estimator.estimateMainProjectionInScreen(
                spot_projections_in_screen
            )
        )

        main_projection_in_screen = LPoint3(
            main_projection_estimation_in_screen[0],
            0,
            main_projection_estimation_in_screen[1],
        )

        return norm(main_projection_in_screen)

    def minimizeProjectionDistanceFromTarget(self, estimate_main_projection_from_spots):
        TOLERANCE = 0.005

        # Get current mirror orientation as fallback in case of minimization error
        (
            initial_mirror_head,
            initial_mirror_pitch,
        ) = self.mirror_panel.getMirrorOrientation()

        if estimate_main_projection_from_spots:
            # 'COBYLA' seems to fall less often in local minimas
            # or being 'stucked' to bounds (but 'Nelder-Mead' and 'Powell' are)
            res = minimize(
                fun=self.estimateMainProjectionErrorFromSpots,
                x0=np.array([initial_mirror_head, initial_mirror_pitch]),
                method="COBYLA",
                tol=TOLERANCE,
                options={"disp": False},
            )
        else:
            # 'COBYLA' seems to fall less often in local minimas
            # or being 'stucked' to bounds (but 'Nelder-Mead' and 'Powell' are)
            res = minimize(
                fun=self.computeMainProjectionError,
                x0=np.array([initial_mirror_head, initial_mirror_pitch]),
                method="COBYLA",
                tol=TOLERANCE,
                options={"disp": False},
            )

        # 2*TOLERANCE because 'not precisely guaranteed' according to the COBYLA doc
        if res.success and res.fun < 2 * TOLERANCE:
            self.mirror_panel.setMirrorOrientationWithOffset(res.x[0], res.x[1])
        else:
            # keep the initial orientation
            self.mirror_panel.setMirrorOrientationWithOffset(
                initial_mirror_head, initial_mirror_pitch
            )
            print(res, flush=True)
