// Copyright (C) 2024 Rémi Peuchot
// This code is distributed under GNU GPL v3 license

#pragma once

#include "motors_direction.hpp"

const char *supervisor_get_state(); // for display and debug only

void supervisor_init();

void supervisor_start_manual_move_continuous(motors_direction_t direction);

void supervisor_start_manual_move_one_step(motors_direction_t direction);

void supervisor_start_sun_tracking();

void supervisor_stop();
