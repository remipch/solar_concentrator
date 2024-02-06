#pragma once

#include "motors_direction.hpp"
#include "panel.hpp"

// callback called when motors pass from moving to stopped
// (not called repetitively while motors stay stopped)
typedef void (*motors_stopped_callback)();

void motors_register_stopped_callback(motors_stopped_callback callback);

const char *motors_get_state(); // for display and debug only

void motors_init();

void motors_start_move_continuous(panel_t panel, motors_direction_t direction);

void motors_start_move_one_step(panel_t panel, motors_direction_t direction);

void motors_stop();
