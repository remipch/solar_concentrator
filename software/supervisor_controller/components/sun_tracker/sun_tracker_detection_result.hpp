// Copyright (C) 2024 Rémi Peuchot (https://remipch.github.io/)
// This code is distributed under GNU GPL v3 license

#pragma once

#include <assert.h>

enum class sun_tracker_detection_result_t : signed char {
    UNKNOWN,
    TARGET_NOT_DETECTED,
    SPOT_NOT_DETECTED,
    SPOT_TOO_SMALL,
    SUCCESS,
};

inline const char *str(sun_tracker_detection_result_t state)
{
    switch (state) {
    case sun_tracker_detection_result_t::UNKNOWN:
        return "UNKNOWN";
    case sun_tracker_detection_result_t::TARGET_NOT_DETECTED:
        return "TARGET_NOT_DETECTED";
    case sun_tracker_detection_result_t::SPOT_NOT_DETECTED:
        return "SPOT_NOT_DETECTED";
    case sun_tracker_detection_result_t::SPOT_TOO_SMALL:
        return "SPOT_TOO_SMALL";
    case sun_tracker_detection_result_t::SUCCESS:
        return "SUCCESS";
    default:
        assert(false);
    }
}
