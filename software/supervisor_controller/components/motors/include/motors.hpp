#pragma once

#include "motors_types.hpp"

typedef void (*motors_state_changed_callback)(motors_state_t state);

motors_state_t motors_get_state(); // for display and debug only

void motors_register_state_changed_callback(motors_state_changed_callback state_changed_callback);

void motors_init();

void motors_start_move_continuous(motors_direction_t direction);

void motors_start_move_one_step(motors_direction_t direction);

void motors_stop();
