#pragma once

#include <assert.h>

enum class sun_tracker_logic_result_t : signed char {
    UNKNOWN,
    NO_SPOT_DETECTED,
    SPOT_TOO_SMALL,
    SPOT_TOO_BIG,
    MUST_MOVE,
    TARGET_REACHED,
};

inline const char *str(sun_tracker_logic_result_t state)
{
    switch (state) {
    case sun_tracker_logic_result_t::UNKNOWN:
        return "UNKNOWN";
    case sun_tracker_logic_result_t::NO_SPOT_DETECTED:
        return "NO_SPOT_DETECTED";
    case sun_tracker_logic_result_t::SPOT_TOO_SMALL:
        return "SPOT_TOO_SMALL";
    case sun_tracker_logic_result_t::SPOT_TOO_BIG:
        return "SPOT_TOO_BIG";
    case sun_tracker_logic_result_t::MUST_MOVE:
        return "MUST_MOVE";
    case sun_tracker_logic_result_t::TARGET_REACHED:
        return "TARGET_REACHED";
    default:
        assert(false);
    }
}
