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

#define CHECK_HW(hw_call) CHECK(((hw_call) == motor_hw_error_t::NO_ERROR))

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
    CHECK_HW(motors_hw_init());
    current_status = {
        .state = motors_state_t::IDLE,
        .direction = motors_direction_t::NONE,
        .current = motors_current_t::UNKNWON,
    };
    relaxing_duration_ms = relaxing_phase_duration_ms;
    relaxing_end_time_ms = 0;
}

void start_relaxing_phase(motors_direction_t direction)
{
    if (direction == motors_direction_t::UP_RIGHT) {
        CHECK_HW(motors_hw_start(motor_hw_motor_id_t::DOWN_LEFT, motor_hw_command_t::UNROLL));
    } else if (direction == motors_direction_t::RIGHT) {
        CHECK_HW(motors_hw_start(motor_hw_motor_id_t::DOWN_LEFT, motor_hw_command_t::UNROLL));
    } else if (direction == motors_direction_t::DOWN_RIGHT) {
        CHECK_HW(motors_hw_start(motor_hw_motor_id_t::UP, motor_hw_command_t::UNROLL));
    } else if (direction == motors_direction_t::DOWN_LEFT) {
        CHECK_HW(motors_hw_start(motor_hw_motor_id_t::UP, motor_hw_command_t::UNROLL));
    } else if (direction == motors_direction_t::LEFT) {
        CHECK_HW(motors_hw_start(motor_hw_motor_id_t::DOWN_RIGHT, motor_hw_command_t::UNROLL));
    } else if (direction == motors_direction_t::UP_LEFT) {
        CHECK_HW(motors_hw_start(motor_hw_motor_id_t::DOWN_RIGHT, motor_hw_command_t::UNROLL));
    } else {
        motors_logic_treat_error("invalid direction");
    }
}

void start_tensing_phase(motors_direction_t direction)
{
    if (direction == motors_direction_t::UP_RIGHT) {
        CHECK_HW(motors_hw_start(motor_hw_motor_id_t::UP, motor_hw_command_t::ROLL));
    } else if (direction == motors_direction_t::RIGHT) {
        CHECK_HW(motors_hw_start(motor_hw_motor_id_t::DOWN_RIGHT, motor_hw_command_t::ROLL));
    } else if (direction == motors_direction_t::DOWN_RIGHT) {
        CHECK_HW(motors_hw_start(motor_hw_motor_id_t::DOWN_RIGHT, motor_hw_command_t::ROLL));
    } else if (direction == motors_direction_t::DOWN_LEFT) {
        CHECK_HW(motors_hw_start(motor_hw_motor_id_t::DOWN_LEFT, motor_hw_command_t::ROLL));
    } else if (direction == motors_direction_t::LEFT) {
        CHECK_HW(motors_hw_start(motor_hw_motor_id_t::DOWN_LEFT, motor_hw_command_t::ROLL));
    } else if (direction == motors_direction_t::UP_LEFT) {
        CHECK_HW(motors_hw_start(motor_hw_motor_id_t::UP, motor_hw_command_t::ROLL));
    } else {
        motors_logic_treat_error("invalid direction");
    }
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
    start_relaxing_phase(direction);
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
    start_relaxing_phase(direction);
}

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
    CHECK_HW(motors_hw_stop());
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
    motors_current_t measured_current = motor_hw_measure_current();
    ESP_LOGV(TAG, "measured_current = %s", str(measured_current));

    if (current_status.state == motors_state_t::MOVING || current_status.state == motors_state_t::MOVING_ONE_STEP) {
        if (relaxing_end_time_ms != 0) {
            if (time_since_boot_ms >= relaxing_end_time_ms) {
                ESP_LOGD(TAG, "End relaxing phase");
                if (measured_current == motors_current_t::HIGH) {
                    motors_logic_treat_error("current is still high after relaxing phase");
                    return;
                }
                relaxing_end_time_ms = 0;
                start_tensing_phase(current_status.direction);
            }
        } else if (measured_current == motors_current_t::HIGH) {
            if (current_status.state == motors_state_t::MOVING_ONE_STEP) {
                motors_hw_stop();
                current_status = {
                    .state = motors_state_t::LOCKED,
                    .direction = motors_direction_t::NONE,
                    .current = motors_current_t::UNKNWON,
                };
                return;
            } else {
                relaxing_end_time_ms = time_since_boot_ms + relaxing_duration_ms;
                ESP_LOGD(TAG, "Start relaxing phase (relaxing_end_time_ms = %i)", relaxing_end_time_ms);
                start_relaxing_phase(current_status.direction);
            }
        }
    }
    current_status.current = measured_current;
}
