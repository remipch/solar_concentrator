#pragma once

#include "sun_tracker_callbacks.hpp"

void sun_tracker_register_result_callback(sun_tracker_result_callback callback);

void sun_tracker_register_full_image_callback(sun_tracker_image_callback callback);

void sun_tracker_register_target_image_callback(sun_tracker_image_callback callback);

const char* sun_tracker_get_state(); // for display and debug only

void sun_tracker_init();

void sun_tracker_start();

void sun_tracker_reset();
