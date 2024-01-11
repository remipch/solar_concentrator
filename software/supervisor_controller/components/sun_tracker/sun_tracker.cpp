#include "sun_tracker.hpp"
#include "sun_tracker_state_machine.hpp"

#include "esp_log.h"

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

static const char *TAG = "sun_tracker";

// This file is the public interface of the sun_tracker component
// It does not implement logic by itself but provides thread safety to state_machine layer
// It's accomplished by :
// - preventing multiple simultaneous calls to state_machine functions
//   by calling them from a dedicated task
// - protecting state and transition values with a mutex
static const int STATE_MUTEX_TIMEOUT_MS = 100;
static SemaphoreHandle_t state_mutex;
static const int INTER_UPDATE_DELAY_MS = 100;
static sun_tracker_state_t current_state = sun_tracker_state_t::UNINITIALIZED;
static sun_tracker_transition_t asked_transition = sun_tracker_transition_t::NONE;
static sun_tracker_result_callback result_callback = NULL;
static sun_tracker_image_callback full_image_callback = NULL;
static sun_tracker_image_callback target_image_callback = NULL;

void sun_tracker_register_result_callback(sun_tracker_result_callback callback)
{
    // Don't need multiple callbacks for now, a single pointer is enough
    assert(result_callback == NULL);
    result_callback = callback;
}

void sun_tracker_register_full_image_callback(sun_tracker_image_callback callback)
{
    // Don't need multiple callbacks for now, a single pointer is enough
    assert(full_image_callback == NULL);
    full_image_callback = callback;
}

void sun_tracker_register_target_image_callback(sun_tracker_image_callback callback)
{
    // Don't need multiple callbacks for now, a single pointer is enough
    assert(target_image_callback == NULL);
    target_image_callback = callback;
}

void publish_result(sun_tracker_result_t result)
{
    if (result_callback != NULL) {
        result_callback(result);
    }
}

void publish_full_image(CImg<unsigned char> &full_image)
{
    if (full_image_callback != NULL) {
        full_image_callback(full_image);
    }
}

void publish_target_image(CImg<unsigned char> &target_image)
{
    if (target_image_callback != NULL) {
        target_image_callback(target_image);
    }
}

// This function must not be called from an ISR (interrupt service routine)
// because mutex does not support it. Neither ESP32 doc nor FreeRTOS doc is clear
// about what happens in this case, various forums seem to indicate that an 'abort()'
// is triggered with an explanation message.
const char *sun_tracker_get_state()
{
    assert(xSemaphoreTake(state_mutex, pdMS_TO_TICKS(STATE_MUTEX_TIMEOUT_MS)));
    auto state = current_state;
    xSemaphoreGive(state_mutex);
    return str(state);
}

// Note : transition will be reset if state changes after this call
void set_transition(sun_tracker_transition_t transition)
{
    assert(xSemaphoreTake(state_mutex, pdMS_TO_TICKS(STATE_MUTEX_TIMEOUT_MS)));
    asked_transition |= transition;
    xSemaphoreGive(state_mutex);
}

static void sun_tracker_task(void *arg)
{
    while (true) {
        assert(xSemaphoreTake(state_mutex, pdMS_TO_TICKS(STATE_MUTEX_TIMEOUT_MS)));
        sun_tracker_state_t state = current_state;
        sun_tracker_transition_t transition = asked_transition;

        // Reset asked transition :
        // - it will be "consumed" by 'state_machine_update' outside of the mutex guard
        // - one transition must not be treated multiple times by 'state_machine_update'
        // - we allow another transition to be set while 'state_machine_update' is running
        asked_transition = sun_tracker_transition_t::NONE;
        xSemaphoreGive(state_mutex);

        sun_tracker_state_t new_state =
            sun_tracker_state_machine_update(state, transition, publish_full_image, publish_target_image);

        if (new_state != state) {
            ESP_LOGI(
                TAG, "update(state: %s, transition: %s) -> new_state: %s", str(state), str(transition), str(new_state));
        }

        assert(xSemaphoreTake(state_mutex, pdMS_TO_TICKS(STATE_MUTEX_TIMEOUT_MS)));
        current_state = new_state;
        // TODO: call publish_result callback depending on state change (same than motors)
        xSemaphoreGive(state_mutex);

        // Simple wait between state updates because :
        // - don't need a strict period between state updates (don't use periodic timer)
        // - don't need to treat event as fast as possible
        vTaskDelay(pdMS_TO_TICKS(INTER_UPDATE_DELAY_MS));
    }
}

void sun_tracker_init()
{
    ESP_LOGD(TAG, "sun_tracker_init");

    assert(current_state == sun_tracker_state_t::UNINITIALIZED);

    state_mutex = xSemaphoreCreateMutex();

    xTaskCreate(sun_tracker_task, TAG, 4 * 1024, NULL, 5, NULL);
}

void sun_tracker_start() { set_transition(sun_tracker_transition_t::START); }

void sun_tracker_reset() { set_transition(sun_tracker_transition_t::RESET); }
