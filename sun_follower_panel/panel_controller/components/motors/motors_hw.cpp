#include "motors_hw.hpp"

#include "driver/gpio.h"

#include "esp_log.h"
static const char* TAG = "motors_hw";

#define GPIO_MOTOR_DIRECTION_PIN    GPIO_NUM_12
#define GPIO_MOTOR_SELECT_0_PIN     GPIO_NUM_15
#define GPIO_MOTOR_SELECT_1_PIN     GPIO_NUM_13
#define GPIO_ALL_MOTOR_PINS       ( (1ULL << GPIO_MOTOR_DIRECTION_PIN) | (1ULL << GPIO_MOTOR_SELECT_0_PIN) | (1ULL << GPIO_MOTOR_SELECT_1_PIN) )

// Note : because it's impossible to use ADC and wifi simultaneously,
// the current is not measured with an ADC but with 1 digital IO pins associated
// with an external low-pass filter + threshold comparator
// Note : a lot of GPIO pins are used by other components on ESP32-CAM (camera and PSRAM)
// Note : GPIO_NUM_2 pin has an hard-wires 47k pull-up resistor on ESP32-CAM board
#define GPIO_HIGH_CURRENT_PIN   GPIO_NUM_2 // pin is high if current > high threshold

motor_hw_error_t motors_hw_init()
{
    ESP_LOGD(TAG, "motors_hw_init");

    // Config GPIO output for motors
    gpio_config_t io_conf = {};
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pin_bit_mask = GPIO_ALL_MOTOR_PINS;
    io_conf.pull_down_en = gpio_pulldown_t::GPIO_PULLDOWN_DISABLE;
    io_conf.pull_up_en = gpio_pullup_t::GPIO_PULLUP_DISABLE;
    if (gpio_config(&io_conf) != ESP_OK)
        return motor_hw_error_t::CANNOT_USE_GPIO;

    // Configure GPIO inputs for current measuring
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pin_bit_mask = 1ULL << GPIO_HIGH_CURRENT_PIN;
    if (gpio_config(&io_conf) != ESP_OK)
        return motor_hw_error_t::CANNOT_USE_GPIO;

    return motor_hw_error_t::NO_ERROR;
}

#define CHECK_SET_GPIO(pin,value) { \
        if (gpio_set_level(pin, value) != ESP_OK) \
            return motor_hw_error_t::CANNOT_USE_GPIO; \
    }

motor_hw_error_t motors_hw_stop()
{
    CHECK_SET_GPIO(GPIO_MOTOR_SELECT_0_PIN, 0);
    CHECK_SET_GPIO(GPIO_MOTOR_SELECT_1_PIN, 0);
    return motor_hw_error_t::NO_ERROR;
}

motor_hw_error_t motors_hw_start(
    motor_hw_motor_id_t motor_id,
    motor_hw_command_t command
)
{
    ESP_LOGI(TAG, "motors_hw_start(motor_id = %s, command = %s)", str(motor_id), str(command));
    if (motor_id == motor_hw_motor_id_t::UP) {
        CHECK_SET_GPIO(GPIO_MOTOR_SELECT_0_PIN, 1);
        CHECK_SET_GPIO(GPIO_MOTOR_SELECT_1_PIN, 0);
    } else if (motor_id == motor_hw_motor_id_t::DOWN_RIGHT) {
        CHECK_SET_GPIO(GPIO_MOTOR_SELECT_0_PIN, 0);
        CHECK_SET_GPIO(GPIO_MOTOR_SELECT_1_PIN, 1);
    } else if (motor_id == motor_hw_motor_id_t::DOWN_LEFT) {
        CHECK_SET_GPIO(GPIO_MOTOR_SELECT_0_PIN, 1);
        CHECK_SET_GPIO(GPIO_MOTOR_SELECT_1_PIN, 1);
    } else {
        abort();
    }
    if (command == motor_hw_command_t::ROLL) {
        CHECK_SET_GPIO(GPIO_MOTOR_DIRECTION_PIN, 1);
    } else if (command == motor_hw_command_t::UNROLL) {
        CHECK_SET_GPIO(GPIO_MOTOR_DIRECTION_PIN, 0);
    } else {
        abort();
    }
    return motor_hw_error_t::NO_ERROR;
}

motors_current_t motor_hw_measure_current()
{
    if (gpio_get_level(GPIO_HIGH_CURRENT_PIN))
        return motors_current_t::HIGH;
    else
        return motors_current_t::LOW;
}

