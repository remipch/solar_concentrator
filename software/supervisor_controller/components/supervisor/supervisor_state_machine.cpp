#include "supervisor_state_machine.hpp"

#include "esp_log.h" // replaced by stub/esp_log.h for tests on host

#include "motors.hpp"
#include "sun_tracker.hpp"

static const char *TAG = "supervisor_state_machine";

static const int64_t WAITING_SUN_MOVE_DURATION_MS = 10000;

static int64_t start_waiting_time_ms = 0;

supervisor_state_t supervisor_state_machine_update(supervisor_state_t current_state,
                                                   supervisor_transition_t transition,
                                                   motors_direction_t motors_direction,
                                                   int64_t time_ms)
{

    if (current_state == supervisor_state_t::UNINITIALIZED) {
        // Nothing to do but signal state_machine has started by quitting UNINITIALIZED state
        return supervisor_state_t::IDLE;
    }

    if (current_state == supervisor_state_t::IDLE || current_state == supervisor_state_t::MANUAL_MOVING) {
        if (transition == supervisor_transition_t::START_MANUAL_MOVE_ONE_STEP) {
            motors_start_move_one_step(motors_direction);
            return supervisor_state_t::MANUAL_MOVING;
        } else if (transition == supervisor_transition_t::START_MANUAL_MOVE_CONTINUOUS) {
            motors_start_move_continuous(motors_direction);
            return supervisor_state_t::MANUAL_MOVING;
        } else if (transition == supervisor_transition_t::START_SUN_TRACKING) {
            sun_tracker_start();
            return supervisor_state_t::SUN_TRACKING;
        }
    }

    if (current_state == supervisor_state_t::MANUAL_MOVING) {
        if (transition == supervisor_transition_t::STOP_OR_RESET) {
            motors_stop();
            // Stay in same state until MOTORS_STOPPED transition is treated
            return current_state;
        } else if (transition == supervisor_transition_t::MOTORS_STOPPED) {
            return supervisor_state_t::IDLE;
        }
    }

    if (current_state == supervisor_state_t::SUN_TRACKING) {
        if (transition == supervisor_transition_t::STOP_OR_RESET) {
            sun_tracker_stop();
            // Stay in same state until SUN_TRACKING transition is treated
            return current_state;
        } else if (transition == supervisor_transition_t::SUN_TRACKING_ERROR) {
            return supervisor_state_t::ERROR;
        } else if (transition == supervisor_transition_t::SUN_TRACKING_ABORTED) {
            return supervisor_state_t::IDLE;
        } else if (transition == supervisor_transition_t::SUN_TRACKING_SUCCESS) {
            start_waiting_time_ms = time_ms;
            return supervisor_state_t::WAITING_SUN_MOVE;
        }
    }

    if (current_state == supervisor_state_t::WAITING_SUN_MOVE) {
        if (transition == supervisor_transition_t::STOP_OR_RESET) {
            return supervisor_state_t::IDLE;
        } else if ((time_ms - start_waiting_time_ms) > WAITING_SUN_MOVE_DURATION_MS) {
            sun_tracker_start();
            return supervisor_state_t::SUN_TRACKING;
        }
    }

    if (current_state == supervisor_state_t::ERROR) {
        if (transition == supervisor_transition_t::STOP_OR_RESET) {
            // Reset error and go back to IDLE
            sun_tracker_reset();
            return supervisor_state_t::IDLE;
        }
    }

    if (transition != supervisor_transition_t::NONE) {
        ESP_LOGW(TAG, "Nothing to do for state: '%s', transition: '%s'", str(current_state), str(transition));
    }
    return current_state;
}
