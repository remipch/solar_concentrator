// Copyright (C) 2024 Rémi Peuchot
// This code is distributed under GNU GPL v3 license

#include "supervisor.hpp"
#include "motors.hpp"
#include "sun_tracker.hpp"
#include "supervisor_state_machine.hpp"

#include "esp_log.h"
#include "esp_timer.h"

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

static const char *TAG = "supervisor";

// This file is the public interface of the supervisor component
// It does not implement logic by itself but provides thread safety to state_machine layer
// It's accomplished by :
// - preventing multiple simultaneous calls to state_machine functions
//   by calling them from a dedicated task
// - protecting state and transition values with a mutex
static const int STATE_MUTEX_TIMEOUT_MS = 100;
static SemaphoreHandle_t state_mutex;
static const int INTER_UPDATE_DELAY_MS = 100;
static supervisor_state_t current_state = supervisor_state_t::UNINITIALIZED;
static supervisor_transition_t asked_transition = supervisor_transition_t::NONE;
static motors_direction_t asked_direction = motors_direction_t::NONE;

// This function must not be called from an ISR (interrupt service routine)
// because mutex does not support it. Neither ESP32 doc nor FreeRTOS doc is clear
// about what happens in this case, various forums seem to indicate that an 'abort()'
// is triggered with an explanation message.
const char *supervisor_get_state()
{
    assert(xSemaphoreTake(state_mutex, pdMS_TO_TICKS(STATE_MUTEX_TIMEOUT_MS)));
    auto state = current_state;
    xSemaphoreGive(state_mutex);
    return str(state);
}

// Note : transition will be reset if state changes after this call
void set_transition(supervisor_transition_t transition, motors_direction_t direction = motors_direction_t::NONE)
{
    assert(xSemaphoreTake(state_mutex, pdMS_TO_TICKS(STATE_MUTEX_TIMEOUT_MS)));
    if (asked_transition != supervisor_transition_t::NONE) {
        ESP_LOGW(
            TAG,
            "Old transition '%s' ignored because new transition '%s' is asked before the old transition processing",
            str(asked_transition),
            str(transition));
    }
    asked_transition = transition;
    asked_direction = direction;
    xSemaphoreGive(state_mutex);
}

static void supervisor_task(void *arg)
{
    while (true) {
        assert(xSemaphoreTake(state_mutex, pdMS_TO_TICKS(STATE_MUTEX_TIMEOUT_MS)));
        supervisor_state_t state = current_state;
        supervisor_transition_t transition = asked_transition;
        motors_direction_t direction = asked_direction;

        // Reset asked transition :
        // - it will be "consumed" by 'state_machine_update' outside of the mutex guard
        // - one transition must not be treated multiple times by 'state_machine_update'
        // - we allow another transition to be set while 'state_machine_update' is running
        asked_transition = supervisor_transition_t::NONE;
        asked_direction = motors_direction_t::NONE;
        xSemaphoreGive(state_mutex);

        auto time_ms = esp_timer_get_time() / 1000L;

        supervisor_state_t new_state = supervisor_state_machine_update(state, transition, direction, time_ms);

        if (new_state != state) {
            ESP_LOGI(TAG,
                     "update(state: %s, transition: %s, direction: %s) -> new_state: %s",
                     str(state),
                     str(transition),
                     str(direction),
                     str(new_state));
        }

        assert(xSemaphoreTake(state_mutex, pdMS_TO_TICKS(STATE_MUTEX_TIMEOUT_MS)));
        current_state = new_state;
        xSemaphoreGive(state_mutex);

        // Simple wait between state updates because :
        // - don't need a strict period between state updates (don't use periodic timer)
        // - don't need to treat event as fast as possible
        vTaskDelay(pdMS_TO_TICKS(INTER_UPDATE_DELAY_MS));
    }
}

// Called by motors when motors just stopped
void motors_stopped() { set_transition(supervisor_transition_t::MOTORS_STOPPED); }

void sun_tracker_result(sun_tracker_result_t result)
{
    if (result == sun_tracker_result_t::ERROR) {
        set_transition(supervisor_transition_t::SUN_TRACKING_ERROR);
    } else if (result == sun_tracker_result_t::MAX_MOVES) {
        set_transition(supervisor_transition_t::SUN_TRACKING_MAX_MOVES);
    } else if (result == sun_tracker_result_t::ABORTED) {
        set_transition(supervisor_transition_t::SUN_TRACKING_ABORTED);
    } else if (result == sun_tracker_result_t::SUCCESS) {
        set_transition(supervisor_transition_t::SUN_TRACKING_SUCCESS);
    }
}

void supervisor_init()
{
    ESP_LOGD(TAG, "supervisor_init");

    assert(current_state == supervisor_state_t::UNINITIALIZED);
    motors_register_stopped_callback(motors_stopped);
    sun_tracker_register_result_callback(sun_tracker_result);
    state_mutex = xSemaphoreCreateMutex();
    xTaskCreate(supervisor_task, TAG, 4 * 1024, NULL, 5, NULL);
}

void supervisor_stop() { set_transition(supervisor_transition_t::STOP_OR_RESET); }

void supervisor_start_manual_move_continuous(motors_direction_t direction)
{
    set_transition(supervisor_transition_t::START_MANUAL_MOVE_CONTINUOUS, direction);
}

void supervisor_start_manual_move_one_step(motors_direction_t direction)
{
    set_transition(supervisor_transition_t::START_MANUAL_MOVE_ONE_STEP, direction);
}

void supervisor_start_sun_tracking() { set_transition(supervisor_transition_t::START_SUN_TRACKING); }
