#pragma once

#include "motors_types.hpp"

motors_full_status_t motors_get_status();

// Must be called once before any other functions
void motors_init();

void motors_start_move(motors_direction_t direction);

void motors_start_move_one_step(motors_direction_t direction);

// Move one step and wait motors to finish their move.
// It's the only synchronous function, other functions push
// an event and returns immediately
void motors_blocking_move_one_step(motors_direction_t direction);

void motors_stop();
