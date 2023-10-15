#include "motors_state_machine.hpp"
#include "motors_hw.hpp"

#include "esp_log.h" // replaced by stub/esp_log.h for tests on host

#include <assert.h>

static const char* TAG = "motors_state_machine";

motors_state_t motors_state_machine_update(
    motors_state_t current_state,
    motors_transition_t transition,
    motors_direction_t motors_direction) {

    static motors_direction_t continuous_motors_direction = motors_direction_t::NONE;

    if(current_state==motors_state_t::UNINITIALIZED) {
        assert(motors_hw_init()== motor_hw_error_t::NO_ERROR);
        return motors_state_t::STOPPED;
    }

    // Transitions can be applied to any state,
    // treat them globally outside of 'if(state==...){}' blocs to avoid repetition
    if(transition==motors_transition_t::START_MOVE_CONTINUOUS) {
        continuous_motors_direction = motors_direction;
        motors_hw_move_one_step(motors_direction);
        return motors_state_t::MOVING_CONTINUOUS;
    }
    else if(transition==motors_transition_t::START_MOVE_ONE_STEP) {
        motors_hw_move_one_step(motors_direction);
        return motors_state_t::MOVING_ONE_STEP;
    }
    else if(transition==motors_transition_t::STOP) {
        motors_hw_stop();
        return motors_state_t::STOPPING;
    }

    if(current_state==motors_state_t::MOVING_CONTINUOUS) {
        if(motor_hw_get_state()==motor_hw_state_t::STOPPED) {
            // Restart move continuously
            motors_hw_move_one_step(continuous_motors_direction);
            return motors_state_t::MOVING_CONTINUOUS;
        }
        else {
            // Stay in same state while motors are moving
            return current_state;
        }
    }
    else { // STOPPING and MOVING_ONE_STEP states are treated the same way
        if(motor_hw_get_state()==motor_hw_state_t::STOPPED) {
            return motors_state_t::STOPPED;
        }
        else {
            // Stay in same state while motors are moving
            return current_state;
        }
    }

    if(transition!=motors_transition_t::NONE) {
        ESP_LOGW(TAG, "Nothing to do for state: '%s', transition: '%s'",
                 str(current_state), str(transition));
    }
    return current_state;
}

