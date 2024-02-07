#include "supervisor_state_machine.hpp"

#include "esp_log.h" // replaced by stub/esp_log.h for tests on host

#include "motors.hpp"
#include "sun_tracker.hpp"

#include <assert.h>

static const char *TAG = "supervisor_state_machine";

static const int64_t WAITING_SUN_MOVE_DURATION_MS = 10000;

static const int MAX_RETRY_ON_ERROR = 10; // Go to 'ERROR' state after that

static int64_t start_waiting_time_ms = 0;

static int retry_count = 0;

panel_t get_next_panel(panel_t panel)
{
    ESP_LOGD(TAG, "get_next_panel(%s)", str(panel));
    if (panel == panel_t::PANEL_A) {
        return panel_t::PANEL_B;
    } else if (panel == panel_t::PANEL_B) {
        return panel_t::PANEL_A;
    } else {
        assert(false);
    }
}

supervisor_state_t supervisor_state_machine_update(supervisor_state_t current_state,
                                                   supervisor_transition_t transition,
                                                   panel_t &panel,
                                                   motors_direction_t motors_direction,
                                                   int64_t time_ms)
{
    if (current_state == supervisor_state_t::UNINITIALIZED) {
        // Nothing to do but signal state_machine has started by quitting UNINITIALIZED state
        return supervisor_state_t::IDLE;
    }

    if (current_state == supervisor_state_t::IDLE || current_state == supervisor_state_t::MANUAL_MOVING) {
        if (transition == supervisor_transition_t::START_MANUAL_MOVE_ONE_STEP) {
            motors_start_move_one_step(panel, motors_direction);
            return supervisor_state_t::MANUAL_MOVING;
        } else if (transition == supervisor_transition_t::START_MANUAL_MOVE_CONTINUOUS) {
            motors_start_move_continuous(panel, motors_direction);
            return supervisor_state_t::MANUAL_MOVING;
        } else if (transition == supervisor_transition_t::START_SUN_TRACKING) {
            retry_count = 0;
            sun_tracker_start(panel);
            return supervisor_state_t::SUN_TRACKING;
        }
    }

    if (current_state == supervisor_state_t::IDLE) {
        if (transition == supervisor_transition_t::ACTIVATE_NEXT_PANEL) {
            panel = get_next_panel(panel);
            return current_state;
        }
    }

    if (current_state == supervisor_state_t::MANUAL_MOVING) {
        if (transition == supervisor_transition_t::STOP_OR_RESET) {
            motors_stop();
            // Stay in same state until MOTORS_STOPPED transition is treated
            return current_state;
        } else if (transition == supervisor_transition_t::ACTIVATE_NEXT_PANEL) {
            panel = get_next_panel(panel);
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
        } else if (transition == supervisor_transition_t::SUN_TRACKING_MAX_MOVES) {
            ESP_LOGE(TAG, "SUN_TRACKING_MAX_MOVES: go to error state immediately");
            return supervisor_state_t::ERROR;
        } else if (transition == supervisor_transition_t::SUN_TRACKING_ERROR) {
            ESP_LOGW(TAG, "SUN_TRACKING_ERROR received: retry %i", retry_count);
            if (retry_count++ > MAX_RETRY_ON_ERROR) {
                ESP_LOGE(TAG, "MAX_RETRY_ON_ERROR reached: go to error state");
                return supervisor_state_t::ERROR;
            }

            // Error is often caused by bad image acquisition
            // (auto expo leading to bad capstone detection) or misdetected spot
            // Retrying will help in major cases
            sun_tracker_start(panel);
            return supervisor_state_t::SUN_TRACKING;
        } else if (transition == supervisor_transition_t::SUN_TRACKING_ABORTED) {
            return supervisor_state_t::IDLE;
        } else if (transition == supervisor_transition_t::SUN_TRACKING_SUCCESS) {
            start_waiting_time_ms = time_ms;
            panel = get_next_panel(panel);
            return supervisor_state_t::WAITING_SUN_MOVE;
        }
    }

    if (current_state == supervisor_state_t::WAITING_SUN_MOVE) {
        if (transition == supervisor_transition_t::STOP_OR_RESET) {
            return supervisor_state_t::IDLE;
        } else if ((time_ms - start_waiting_time_ms) > WAITING_SUN_MOVE_DURATION_MS) {
            retry_count = 0;
            sun_tracker_start(panel);
            return supervisor_state_t::SUN_TRACKING;
        }
    }

    if (current_state == supervisor_state_t::ERROR) {
        if (transition == supervisor_transition_t::STOP_OR_RESET) {
            // Reset error and go back to IDLE
            return supervisor_state_t::IDLE;
        }
    }

    return current_state;
}
