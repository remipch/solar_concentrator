/* UART Echo Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/uart.h"
#include "driver/gpio.h"
#include "sdkconfig.h"
#include "esp_log.h"

/**
 * This is an example which echos any data it receives on configured UART back to the sender,
 * with hardware flow control turned off. It does not use UART driver event queue.
 *
 * - Port: configured UART
 * - Receive (Rx) buffer: on
 * - Transmit (Tx) buffer: off
 * - Flow control: off
 * - Event queue: off
 * - Pin assignment: see defines below (See Kconfig)
 */

#define ECHO_TEST_TXD 13
#define ECHO_TEST_RXD 15
#define ECHO_TEST_RTS (UART_PIN_NO_CHANGE)
#define ECHO_TEST_CTS (UART_PIN_NO_CHANGE)

#define ECHO_UART_PORT_NUM      2
#define ECHO_UART_BAUD_RATE     19200

static const char *TAG = "UART TEST";

#define BUF_SIZE (1024)

const char END_CHAR = '\n';

void write_motors_commands(char* commands) {
    int res = uart_write_bytes(ECHO_UART_PORT_NUM, (const char *) commands, strlen(commands));
    res += uart_write_bytes(ECHO_UART_PORT_NUM, &END_CHAR, 1);
    ESP_LOGI(TAG, "commands sent (%i)",res);
}

// return true if last known state is active
bool get_motors_state() {
    write_motors_commands("s");

    char reply[BUF_SIZE];
    int len = uart_read_bytes(ECHO_UART_PORT_NUM, reply, (BUF_SIZE - 1), 20 / portTICK_PERIOD_MS);
    if (len>0) {
        reply[len] = '\0';
        ESP_LOGI(TAG, "Receive reply: %s", (char *) reply);
        if(reply[len-1]=='1')
            return true;
        else if(reply[len-1]=='0')
            return false;
        else
            ESP_LOGW(TAG, "Unkown reply");
            return false;
    }
    else {
        ESP_LOGW(TAG, "No reply");
        return false;
    }
}

void app_main(void)
{
    /* Configure parameters of an UART driver,
     * communication pins and install the driver */
    uart_config_t uart_config = {
        .baud_rate = ECHO_UART_BAUD_RATE,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_APB,
    };
    int intr_alloc_flags = 0;

#if CONFIG_UART_ISR_IN_IRAM
    intr_alloc_flags = ESP_INTR_FLAG_IRAM;
#endif

    ESP_ERROR_CHECK(uart_driver_install(ECHO_UART_PORT_NUM, BUF_SIZE * 2, 0, 0, NULL, intr_alloc_flags));
    ESP_ERROR_CHECK(uart_param_config(ECHO_UART_PORT_NUM, &uart_config));
    ESP_ERROR_CHECK(uart_set_pin(ECHO_UART_PORT_NUM, ECHO_TEST_TXD, ECHO_TEST_RXD, ECHO_TEST_RTS, ECHO_TEST_CTS));

    // Test Rémi
    const TickType_t xDelay = 500 / portTICK_PERIOD_MS;
    vTaskDelay(xDelay);
    get_motors_state();
    vTaskDelay(xDelay);
    write_motors_commands("o:1,1000;o:32,5000,25");
    for(int i=0;i<10;i++) {
        vTaskDelay(xDelay);
        get_motors_state();
    }
}
