#pragma once

#include "motors_types.hpp"

enum class motor_hw_error_t : char {
    NO_ERROR = 0,
    CANNOT_USE_GPIO,
    INVALID_CURRENT,
};

enum class motor_hw_motor_id_t : char {
    UP,
    DOWN_RIGHT,
    DOWN_LEFT,
};

inline const char* str(motor_hw_motor_id_t motor_id)
{
    switch (motor_id) {
    case motor_hw_motor_id_t::UP:
        return "UP";
    case motor_hw_motor_id_t::DOWN_RIGHT:
        return "DOWN_RIGHT";
    case motor_hw_motor_id_t::DOWN_LEFT:
        return "DOWN_LEFT";
    default:
        abort();
    }
}

enum class motor_hw_command_t : char {
    ROLL,
    UNROLL,
};

inline const char* str(motor_hw_command_t command)
{
    switch (command) {
    case motor_hw_command_t::ROLL:
        return "ROLL";
    case motor_hw_command_t::UNROLL:
        return "UNROLL";
    default:
        abort();
    }
}

motor_hw_error_t motors_hw_init();

motor_hw_error_t motors_hw_stop();

motor_hw_error_t motors_hw_start(
    motor_hw_motor_id_t motor_id,
    motor_hw_command_t command
);

motors_current_t motor_hw_measure_current();
