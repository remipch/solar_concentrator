#pragma once

#include "motors_types.hpp"

motors_full_status_t motors_get_status();

void motors_init();

void motors_start_move(motors_direction_t direction);

void motors_start_move_one_step(motors_direction_t direction);

void motors_start_tighten();

void motors_stop();
