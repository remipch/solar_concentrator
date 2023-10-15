#include "supervisor_state_machine.hpp"

#include "esp_log.h" // replaced by stub/esp_log.h for tests on host

#include "motors.hpp"

static const char* TAG = "supervisor_state_machine";

bool can_start_manual_move_from(supervisor_state_t state) {
    return (state==supervisor_state_t::IDLE ||
            state==supervisor_state_t::MANUAL_MOVING ||
            state==supervisor_state_t::WAITING_SUN_MOVE);
}

supervisor_state_t supervisor_state_machine_update(
    supervisor_state_t current_state,
    supervisor_transition_t transition,
    motors_direction_t motors_direction) {

    if(current_state==supervisor_state_t::UNINITIALIZED) {
        // Nothing to do but signal state_machine has started by quitting UNINITIALIZED state
        return supervisor_state_t::IDLE;
    }

    // Manual move transitions can be applied from several states
    // test transition first and current_state secondly
    if(transition==supervisor_transition_t::START_MANUAL_MOVE_ONE_STEP) {
        if(can_start_manual_move_from(current_state)) {
            motors_start_move_one_step(motors_direction);
            return supervisor_state_t::MANUAL_MOVING;
        }
        else {
            ESP_LOGW(TAG, "Cannot start manual move while tracking, stop tracking first");
            return current_state;
        }
    }
    else if(transition==supervisor_transition_t::START_MANUAL_MOVE_CONTINUOUS) {
        if(can_start_manual_move_from(current_state)) {
            motors_start_move_continuous(motors_direction);
            return supervisor_state_t::MANUAL_MOVING;
        }
        else {
            ESP_LOGW(TAG, "Cannot start manual move while tracking, stop tracking first");
            return current_state;
        }
    }

    if(current_state==supervisor_state_t::MANUAL_MOVING) {
        if(transition==supervisor_transition_t::STOP) {
            motors_stop();
            // Stay in same state until MOTORS_STOPPED transition is treated
            return current_state;
        }
        else if(transition==supervisor_transition_t::MOTORS_STOPPED) {
            return supervisor_state_t::IDLE;
        }
    }

    if(transition!=supervisor_transition_t::NONE) {
        ESP_LOGW(TAG, "Nothing to do for state: '%s', transition: '%s'",
                 str(current_state), str(transition));
    }
    return current_state;
}
