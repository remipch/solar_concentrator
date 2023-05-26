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

// Param set at initialization
static int relaxing_duration_ms;

// Set when relaxing phase starts
// 0 if no current relaxing phase
static int relaxing_end_time_ms;

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

void motors_logic_init(int relaxing_phase_duration_ms)
{
    ESP_LOGI(TAG, "motors_logic_init(relaxing_phase_duration_ms = %i)", relaxing_phase_duration_ms);
    CHECK(current_status.state == motors_state_t::UNINITIALIZED);
    CHECK(motors_hw_init()== motor_hw_error_t::NO_ERROR);
    current_status = {
        .state = motors_state_t::IDLE,
        .direction = motors_direction_t::NONE,
        .current = motors_current_t::UNKNWON,
    };
    relaxing_duration_ms = relaxing_phase_duration_ms;
    relaxing_end_time_ms = 0;
}

void motors_logic_start_move(motors_direction_t direction, int time_since_boot_ms)
{
    ESP_LOGI(TAG, "motors_logic_start_move(direction = %s, time_since_boot_ms = %i)",
             str(direction), time_since_boot_ms);
    CHECK(current_status.state > motors_state_t::UNINITIALIZED);
    current_status = {
        .state = motors_state_t::MOVING,
        .direction = direction,
        .current = motors_current_t::UNKNWON, // will be updated by 'motors_logic_periodic_update'
    };
    relaxing_end_time_ms = time_since_boot_ms + relaxing_duration_ms;
    motors_hw_move_one_step(direction);
}

void motors_logic_start_move_one_step(motors_direction_t direction, int time_since_boot_ms)
{
    ESP_LOGI(TAG, "motors_logic_start_move_one_step(direction = %s, time_since_boot_ms = %i)",
             str(direction), time_since_boot_ms);
    CHECK(current_status.state > motors_state_t::UNINITIALIZED);
    current_status = {
        .state = motors_state_t::MOVING_ONE_STEP,
        .direction = direction,
        .current = motors_current_t::UNKNWON, // will be updated by 'motors_logic_periodic_update'
    };
    relaxing_end_time_ms = time_since_boot_ms + relaxing_duration_ms;
    motors_hw_move_one_step(direction);
}

// TODO remove this ?
void motors_logic_start_tighten()
{
    ESP_LOGD(TAG, "motors_logic_start_tighten");
    CHECK(current_status.state > motors_state_t::UNINITIALIZED);
    current_status = {
        .state = motors_state_t::TIGHTENING,
        .direction = motors_direction_t::NONE,
        .current = motors_current_t::UNKNWON, // will be updated by 'motors_logic_periodic_update'
    };
}

void motors_logic_stop()
{
    ESP_LOGI(TAG, "motors_logic_stop");
    CHECK(current_status.state > motors_state_t::UNINITIALIZED);
    motors_hw_stop();
    current_status = {
        .state = motors_state_t::IDLE,
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
    ESP_LOGV(TAG, "state = %s", str(state));
    bool moving = (state != motor_hw_state_t::STOPPED);

    if (current_status.state == motors_state_t::MOVING && !moving) {
        // TODO : start move one more step
        current_status = {
            .state = motors_state_t::LOCKED,
            .direction = motors_direction_t::NONE,
            .current = motors_current_t::UNKNWON,
        };
    }
    else if(current_status.state == motors_state_t::MOVING_ONE_STEP && !moving) {
        current_status = {
            .state = motors_state_t::LOCKED,
            .direction = motors_direction_t::NONE,
            .current = motors_current_t::UNKNWON,
        };
    }
}
