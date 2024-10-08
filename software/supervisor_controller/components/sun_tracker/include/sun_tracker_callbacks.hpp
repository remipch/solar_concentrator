// Copyright (C) 2024 Rémi Peuchot (https://remipch.github.io/)
// This code is distributed under GNU GPL v3 license

#pragma once

#include "image.hpp"

#include <assert.h>
#include <functional>

enum class sun_tracker_result_t : signed char {
    UNKNOWN,
    ERROR,     // Unexpected error, can be solved by retrying a few times
    MAX_MOVES, // Failed after max moves : stop immediately
    ABORTED,
    SUCCESS,
};

inline const char *str(sun_tracker_result_t result)
{
    switch (result) {
    case sun_tracker_result_t::UNKNOWN:
        return "UNKNOWN";
    case sun_tracker_result_t::ERROR:
        return "ERROR";
    case sun_tracker_result_t::MAX_MOVES:
        return "MAX_MOVES";
    case sun_tracker_result_t::ABORTED:
        return "ABORTED";
    case sun_tracker_result_t::SUCCESS:
        return "SUCCESS";
    default:
        assert(false);
    }
}

// Callback called when tracking stopped (for error, succes or interruption)
typedef std::function<void(sun_tracker_result_t)> sun_tracker_result_callback;

// Callback called when full or target image has been updated
// (for debug and display purpose only, the image are processed internally)
// This callback is called from the internal state machine task,
// the given image is guaranteed not to be changed until the callback returns
// Note : full image is GRAYSCALE for optimization (time to capture, less conversions)
// but target image is RGB88 to draw interresting things
// the callback has the responsibility to check image format
typedef std::function<void(CImg<unsigned char> &)> sun_tracker_image_callback;
