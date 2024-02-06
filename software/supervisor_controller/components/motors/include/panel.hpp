#pragma once

#include <assert.h>

// 2 possible panels
enum class panel_t : char {
    NONE = 0,
    PANEL_A,
    PANEL_B,
};

// Convenient function for logging
inline const char *str(panel_t panel)
{
    switch (panel) {
    case panel_t::NONE:
        return "NONE";
    case panel_t::PANEL_A:
        return "PANEL_A";
    case panel_t::PANEL_B:
        return "PANEL_B";
    default:
        assert(false);
    }
}
