#pragma once

#include "motors_direction.hpp"

const char *supervisor_get_state(); // for display and debug only

void supervisor_init();

void supervisor_activate_next_panel();

void supervisor_start_manual_move_continuous(motors_direction_t direction);

void supervisor_start_manual_move_one_step(motors_direction_t direction);

void supervisor_start_sun_tracking();

void supervisor_stop();
