#pragma once

#include "motors_types.hpp"

// None of these functions is thread safe, the caller has the responsibility to :
// - never call multiple functions simultaneously
// - cache motors status to give them (optionaly asynchronously) to external components

motors_full_status_t motors_logic_get_status();

// Must be called once before any other functions
void motors_logic_init();

void motors_logic_start_move(motors_direction_t direction);

void motors_logic_start_move_one_step(motors_direction_t direction);

void motors_logic_stop();

// Must be called periodically to do meaningfull actions depending on current state
// and update internal state
void motors_logic_periodic_update(int time_since_boot_ms);

