#include "motors_logic.hpp"
#include "motors_hw.hpp"

#include "esp_log.h" // replaced by stub/esp_log.h for tests on host

#include <assert.h>

static const char* TAG = "motors_logic";

static motors_full_status_t current_status = {
    .state = motors_state_t::UNINITIALIZED,
    .direction = motors_direction_t::NONE,
    .current = motors_current_t::UNKNWON,
};

// Shortcut to treat errors consistently if condition is false
#define CHECK(condition) \
    if(!(condition)) { \
        motors_logic_treat_error("[" #condition "] not satisfied"); \
        return; \
    }

// Treat errors consistently :
// - log the given error message
// - try to stop motors
// - set current state to error
void motors_logic_treat_error(const char* message)
{
    ESP_LOGE(TAG, "motors_logic error : %s", message);
    if (current_status.state > motors_state_t::UNINITIALIZED) {
        motors_hw_stop();
    }
    current_status = {
        .state = motors_state_t::ERROR,
        .direction = motors_direction_t::NONE,
        .current = motors_current_t::UNKNWON,
    };
}

motors_full_status_t motors_logic_get_status()
{
    return current_status;
}

void motors_logic_init()
{
    esp_log_level_set(TAG, ESP_LOG_DEBUG);
    ESP_LOGI(TAG, "motors_logic_init()");
    CHECK(current_status.state == motors_state_t::UNINITIALIZED);
    CHECK(motors_hw_init()== motor_hw_error_t::NO_ERROR);
    current_status = {
        .state = motors_state_t::STOPPED,
        .direction = motors_direction_t::NONE,
        .current = motors_current_t::UNKNWON,
    };
}

void motors_logic_start_move(motors_direction_t direction, int time_since_boot_ms)
{
    ESP_LOGD(TAG, "motors_logic_start_move(direction = %s, time_since_boot_ms = %i)",
             str(direction), time_since_boot_ms);
    CHECK(current_status.state > motors_state_t::UNINITIALIZED);
    current_status = {
        .state = motors_state_t::MOVING,
        .direction = direction,
        .current = motors_current_t::UNKNWON, // will be updated by 'motors_logic_periodic_update'
    };
    motors_hw_move_one_step(direction);
}

void motors_logic_start_move_one_step(motors_direction_t direction, int time_since_boot_ms)
{
    ESP_LOGD(TAG, "motors_logic_start_move_one_step(direction = %s, time_since_boot_ms = %i)",
             str(direction), time_since_boot_ms);
    CHECK(current_status.state > motors_state_t::UNINITIALIZED);
    current_status = {
        .state = motors_state_t::MOVING_ONE_STEP,
        .direction = direction,
        .current = motors_current_t::UNKNWON, // will be updated by 'motors_logic_periodic_update'
    };
    motors_hw_move_one_step(direction);
}

void motors_logic_stop()
{
    ESP_LOGD(TAG, "motors_logic_stop");
    CHECK(current_status.state > motors_state_t::UNINITIALIZED);
    motors_hw_stop();
    current_status = {
        .state = motors_state_t::STOPPED,
        .direction = motors_direction_t::NONE,
        .current = motors_current_t::UNKNWON, // will be updated by 'motors_logic_periodic_update'
    };
}

void motors_logic_periodic_update(int time_since_boot_ms)
{
    if (current_status.state <= motors_state_t::UNINITIALIZED) {
        // Ignore update if error or not yet initialized
        return;
    }
    ESP_LOGV(TAG, "motors_logic_periodic_update(time_since_boot_ms = %i)", time_since_boot_ms);
    motor_hw_state_t state = motor_hw_get_state();
    //CHECK(state != motor_hw_state_t::UNKNOWN);
    bool moving = (state != motor_hw_state_t::STOPPED);

    if (current_status.state == motors_state_t::MOVING && !moving) {
        motors_hw_move_one_step(current_status.direction);
    }
    else if(current_status.state == motors_state_t::MOVING_ONE_STEP && !moving) {
        current_status = {
            .state = motors_state_t::STOPPED,
            .direction = motors_direction_t::NONE,
            .current = motors_current_t::UNKNWON,
        };
    }
}
