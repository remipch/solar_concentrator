#include "motors.hpp"
#include "motors_logic.hpp"

#include "esp_event.h"
#include "esp_timer.h"
#include "esp_log.h"
#include "esp_timer.h"

#include "freertos/semphr.h"

static const char* TAG = "motors";

// This file has the only responsibility to provide thread safety to motors_logic layer
// It's accomplished by :
// - preventing multiple simultaneous calls to motors_logic functions by calling them
//   from a single-threaded event_loop (native esp_event with its associated task to dispatch events)
// - caching public state values (state, current and direction) and protecting them with a mutex
// These technical choices are not exposed publicly but only kept here internally
static bool is_initialized = false;
static esp_event_loop_handle_t event_loop;
static ESP_EVENT_DEFINE_BASE(MOTORS_EVENTS); // (not declared in header because only used here internally)
static TickType_t EVENT_LOOP_TIMEOUT_MS = 100;
static const uint64_t TIMER_PERIOD_MS = 100;
static esp_timer_handle_t periodic_timer;
static SemaphoreHandle_t status_mutex;

enum {
    MOTORS_INIT_EVENT,
    MOTORS_START_MOVE_EVENT,
    MOTORS_START_MOVE_ONE_STEP_EVENT,
    MOTORS_STOP_EVENT,
    MOTORS_PERIODIC_UPDATE_EVENT,
};

// Cache last values reported by motors_logic
static motors_full_status_t current_status = {
    .state = motors_state_t::UNINITIALIZED,
    .direction = motors_direction_t::NONE,
    .current = motors_current_t::UNKNWON,
};

// This function must not be called from an ISR (interrupt service routine)
// because mutex does not support it. Neither ESP32 doc nor FreeRTOS doc is clear
// about what happens in this case, various forums seem to indicate that an 'abort()'
// is triggered with an explanation message.
motors_full_status_t motors_get_status()
{
    // 'status' is only writen by the event_loop task, it can be read by any other task.
    // 'status_mutex' ensures 'status' variable consistency
    // portMAX_DELAY is used but the code inside the mutex-protected bloc is short
    // and should not run for a long time.
    xSemaphoreTake(status_mutex, portMAX_DELAY);
    auto status = current_status;
    xSemaphoreGive(status_mutex);
    return status;
}

static void motors_event_handler(void* handler_args, esp_event_base_t base, int32_t id, void* event_data)
{
    int64_t time_since_boot_ms = esp_timer_get_time() / 1000;
    switch (id) {
    case MOTORS_INIT_EVENT:
        motors_logic_init();
        break;
    case MOTORS_START_MOVE_EVENT: {
        motors_direction_t direction = *((motors_direction_t*)event_data);
        motors_logic_start_move(direction, time_since_boot_ms);
        break;
    }
    case MOTORS_START_MOVE_ONE_STEP_EVENT: {
        motors_direction_t direction = *((motors_direction_t*)event_data);
        motors_logic_start_move_one_step(direction, time_since_boot_ms);
        break;
    }
    case MOTORS_STOP_EVENT:
        motors_logic_stop();
        break;
    case MOTORS_PERIODIC_UPDATE_EVENT: {
        motors_logic_periodic_update(time_since_boot_ms);
        break;
    }
    default:
        ESP_LOGE(TAG, "Unkown event : ignored");
    }

    xSemaphoreTake(status_mutex, portMAX_DELAY);
    current_status = motors_logic_get_status();
    xSemaphoreGive(status_mutex);
}

void post_event(int32_t event_id, void* data = NULL, size_t data_size = 0)
{
    ESP_ERROR_CHECK(esp_event_post_to(
                        event_loop,
                        MOTORS_EVENTS,
                        event_id,
                        data,
                        data_size,
                        pdMS_TO_TICKS(EVENT_LOOP_TIMEOUT_MS))
                   );
}

static void periodic_timer_callback(void* arg)
{
    post_event(MOTORS_PERIODIC_UPDATE_EVENT);
}

void motors_init()
{
    ESP_LOGD(TAG, "motors_init");

    if (!is_initialized) {
        esp_event_loop_args_t event_loop_args = {
            .queue_size = 5,
            .task_name = "loop_task",
            .task_priority = uxTaskPriorityGet(NULL),
            .task_stack_size = 3072,
            .task_core_id = tskNO_AFFINITY
        };
        ESP_ERROR_CHECK(esp_event_loop_create(&event_loop_args, &event_loop));
        ESP_ERROR_CHECK(esp_event_handler_instance_register_with(
                            event_loop,
                            MOTORS_EVENTS,
                            ESP_EVENT_ANY_ID,
                            motors_event_handler,
                            NULL,
                            NULL));

        const esp_timer_create_args_t periodic_timer_args = {
            .callback = &periodic_timer_callback,
            .arg = NULL,
            .dispatch_method = ESP_TIMER_TASK,
            .name = "motors_periodic_timer",
            .skip_unhandled_events = true,
        };
        ESP_ERROR_CHECK(esp_timer_create(&periodic_timer_args, &periodic_timer));
        ESP_ERROR_CHECK(esp_timer_start_periodic(periodic_timer, TIMER_PERIOD_MS * 1000));

        status_mutex = xSemaphoreCreateMutex();

        is_initialized = true;
    }

    post_event(MOTORS_INIT_EVENT);
}

void motors_start_move(motors_direction_t direction)
{
    post_event(MOTORS_START_MOVE_EVENT, &direction, sizeof(motors_direction_t));
}

void motors_start_move_one_step(motors_direction_t direction)
{
    post_event(MOTORS_START_MOVE_ONE_STEP_EVENT, &direction, sizeof(motors_direction_t));
}

void motors_stop()
{
    post_event(MOTORS_STOP_EVENT);
}

