#include "motors.hpp"
#include "motors_state_machine.hpp"

#include "esp_log.h"

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

static const char* TAG = "motors";

// This file is the public interface of motors component
// It does not implement motors logic by itself but
// provide thread safety to motors_state_machine layer
// It's accomplished by :
// - preventing multiple simultaneous calls to motors_state_machine functions functions
// by calling them from a single-threaded permanent task
// - protecting state and transition values with a mutex
static const int STATE_MUTEX_TIMEOUT_MS = 100;
static SemaphoreHandle_t state_mutex;
static const int INTER_UPDATE_DELAY_MS = 100;
static motors_state_t current_state = motors_state_t::UNINITIALIZED;
static motors_transition_t asked_transition = motors_transition_t::NONE;
static motors_direction_t asked_direction = motors_direction_t::NONE;
static motors_stopped_callback stopped_callback = NULL;

void motors_register_stopped_callback(motors_stopped_callback callback) {
    // Don't need multiple callbacks for now, a single pointer is enough
    assert(stopped_callback==NULL);
    stopped_callback = callback;
}

// This function must not be called from an ISR (interrupt service routine)
// because mutex does not support it. Neither ESP32 doc nor FreeRTOS doc is clear
// about what happens in this case, various forums seem to indicate that an 'abort()'
// is triggered with an explanation message.
const char* motors_get_state()
{
    assert(xSemaphoreTake(state_mutex, pdMS_TO_TICKS(STATE_MUTEX_TIMEOUT_MS)));
    auto state = current_state;
    xSemaphoreGive(state_mutex);
    return str(state);
}

// Note : transition will be reset if state changes after this call
void set_transition(
    motors_transition_t transition,
    motors_direction_t direction = motors_direction_t::NONE)
{
    assert(xSemaphoreTake(state_mutex, pdMS_TO_TICKS(STATE_MUTEX_TIMEOUT_MS)));
    if(asked_transition!=motors_transition_t::NONE) {
        ESP_LOGW(TAG,
                 "Old transition '%s' ignored because new transition '%s' is asked before the old transition processing",
                 str(asked_transition),
                 str(transition));
    }
    asked_transition = transition;
    asked_direction = direction;
    xSemaphoreGive(state_mutex);
}

static void motors_task(void *arg)
{
    while(true) {
        assert(xSemaphoreTake(state_mutex, pdMS_TO_TICKS(STATE_MUTEX_TIMEOUT_MS)));
        motors_state_t state = current_state;
        motors_transition_t transition = asked_transition;
        motors_direction_t direction = asked_direction;

        // Reset asked transition :
        // - it will be "consumed" by 'state_machine_update' outside of the mutex guard
        // - one transition must not be treated multiple times by 'state_machine_update'
        // - we allow another transition to be set while 'state_machine_update' is running
        asked_transition = motors_transition_t::NONE;
        asked_direction = motors_direction_t::NONE;
        xSemaphoreGive(state_mutex);

        motors_state_t new_state = motors_state_machine_update(
            state, transition, direction);

        if(new_state!=state) {
            ESP_LOGI(TAG, "update(state: %s, transition: %s, direction: %s) -> new_state: %s",
                str(state), str(transition), str(direction), str(new_state));
        }

        assert(xSemaphoreTake(state_mutex, pdMS_TO_TICKS(STATE_MUTEX_TIMEOUT_MS)));
        if(stopped_callback!=NULL && current_state!=motors_state_t::STOPPED && new_state==motors_state_t::STOPPED) {
            stopped_callback();
        }
        current_state = new_state;
        xSemaphoreGive(state_mutex);

        // Simple wait between state updates because :
        // - don't need a strict period between state updates (don't use periodic timer)
        // - don't need to treat event as fast as possible
        vTaskDelay(pdMS_TO_TICKS(INTER_UPDATE_DELAY_MS));
    }
}

void motors_init()
{
    ESP_LOGD(TAG, "motors_init");

    assert(current_state == motors_state_t::UNINITIALIZED);

    state_mutex = xSemaphoreCreateMutex();

    xTaskCreate(motors_task, TAG, 4 * 1024, NULL, 5, NULL);
}

void motors_start_move_continuous(motors_direction_t direction)
{
    set_transition(motors_transition_t::START_MOVE_CONTINUOUS, direction);
}

void motors_start_move_one_step(motors_direction_t direction)
{
    set_transition(motors_transition_t::START_MOVE_ONE_STEP, direction);
}

void motors_stop()
{
    set_transition(motors_transition_t::STOP);
}

