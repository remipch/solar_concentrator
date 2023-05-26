#include "motors_hw.hpp"

#include "string.h"
#include "driver/uart.h"
#include "esp_log.h"
static const char* TAG = "motors_hw";

#define MOTOR_CONTROLLER_RX_PIN 15
#define MOTOR_CONTROLLER_TX_PIN 13
#define MOTOR_CONTROLLER_UART_PORT_NUM      2
#define MOTOR_CONTROLLER_BAUD_RATE     19200

#define BUFFER_SIZE (1024)

const char END_CHAR = '\n';

motor_hw_error_t motors_hw_init()
{
    ESP_LOGD(TAG, "motors_hw_init");


    uart_config_t uart_config = {
        .baud_rate = MOTOR_CONTROLLER_BAUD_RATE,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_APB,
    };

    if(uart_driver_install(MOTOR_CONTROLLER_UART_PORT_NUM, BUFFER_SIZE * 2, 0, 0, NULL, 0) != ESP_OK)
        return motor_hw_error_t::CANNOT_USE_UART;
    if(uart_param_config(MOTOR_CONTROLLER_UART_PORT_NUM, &uart_config) != ESP_OK)
        return motor_hw_error_t::CANNOT_USE_UART;
    if(uart_set_pin(MOTOR_CONTROLLER_UART_PORT_NUM, MOTOR_CONTROLLER_TX_PIN, MOTOR_CONTROLLER_RX_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE) != ESP_OK)
        return motor_hw_error_t::CANNOT_USE_UART;

    return motor_hw_error_t::NO_ERROR;
}

bool motors_hw_write_commands(const char* commands) {
    ESP_LOGI(TAG, "Send '%s' to motors",commands);
    int res = uart_write_bytes(MOTOR_CONTROLLER_UART_PORT_NUM, commands, strlen(commands));
    res += uart_write_bytes(MOTOR_CONTROLLER_UART_PORT_NUM, &END_CHAR, 1);
    return (res == strlen(commands)+1);
}

// Return true if the last character received is '1'
char motors_hw_read_reply() {
    char reply[BUFFER_SIZE];
    int len = uart_read_bytes(MOTOR_CONTROLLER_UART_PORT_NUM, reply, (BUFFER_SIZE - 1), 20 / portTICK_PERIOD_MS);
    if (len>0) {
        reply[len] = '\0';
        ESP_LOGI(TAG, "Reply: %s", reply);
        ESP_LOGI(TAG, "Reply: %u", reply[len-1]);
        return reply[len-1];
    }
    else {
        ESP_LOGW(TAG, "No reply");
        return '?';
    }
}

void motors_hw_stop()
{
    motors_hw_write_commands("c");
}

void motors_hw_move_one_step(motors_direction_t direction)
{
    ESP_LOGI(TAG, "motors_hw_move_one_step(direction = %s)", str(direction));
    if (direction == motors_direction_t::UP_RIGHT) {
        motors_hw_write_commands("o:16,1000;o:2,5000,30");
    } else if (direction == motors_direction_t::RIGHT) {
        motors_hw_write_commands("o:16,500;o:8,5000,25");
    } else if (direction == motors_direction_t::DOWN_RIGHT) {
        motors_hw_write_commands("o:1,1000;o:8,5000,25");
    } else if (direction == motors_direction_t::DOWN_LEFT) {
        motors_hw_write_commands("o:1,1000;o:32,5000,25");
    } else if (direction == motors_direction_t::LEFT) {
        motors_hw_write_commands("o:4,500;o:32,5000,25");
    } else if (direction == motors_direction_t::UP_LEFT) {
        motors_hw_write_commands("o:4,1000;o:2,5000,25");
    } else {
        abort();
    }
}

motor_hw_state_t motor_hw_get_state()
{
    motors_hw_write_commands("s");
    char reply = motors_hw_read_reply();
    if(reply=='1')
        return motor_hw_state_t::MOVING;
    if(reply=='0')
        return motor_hw_state_t::STOPPED;
    return motor_hw_state_t::UNKNOWN;
}

