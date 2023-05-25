/* i2c - Simple example

   Simple I2C example that shows how to initialize I2C
   as well as reading and writing from and to registers for a sensor connected over I2C.

   The sensor used in this example is a MPU9250 inertial measurement unit.

   For other examples please check:
   https://github.com/espressif/esp-idf/tree/master/examples

   See README.md file to get detailed usage of this example.

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include <string.h>
#include "esp_log.h"
#include "driver/i2c.h"

static const char *TAG = "i2c-simple-example";

#define I2C_MASTER_SCL_IO           15
#define I2C_MASTER_SDA_IO           13
#define I2C_MASTER_NUM              0                          /*!< I2C master i2c port number, the number of i2c peripheral interfaces available will depend on the chip */
#define I2C_MASTER_FREQ_HZ          400000                     /*!< I2C master clock frequency */
#define I2C_MASTER_TX_BUF_DISABLE   0                          /*!< I2C master doesn't need buffer */
#define I2C_MASTER_RX_BUF_DISABLE   0                          /*!< I2C master doesn't need buffer */
#define I2C_MASTER_TIMEOUT_MS       1000
#define I2C_MASTER_TIMEOUT_TICKS       I2C_MASTER_TIMEOUT_MS / portTICK_PERIOD_MS

#define I2C_SENSOR_ADDR                 0x04

// Read one char from device
static esp_err_t motors_read_state(char *result)
{
    return i2c_master_read_from_device(I2C_MASTER_NUM, I2C_SENSOR_ADDR, (unsigned char*)result, 1, I2C_MASTER_TIMEOUT_TICKS);
}

static esp_err_t motors_write_command(const char* str)
{
    return i2c_master_write_to_device(I2C_MASTER_NUM, I2C_SENSOR_ADDR, (const unsigned char*)str, strlen(str)+1, I2C_MASTER_TIMEOUT_TICKS);
}


static esp_err_t i2c_master_init(void)
{
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = I2C_MASTER_FREQ_HZ,
    };

    i2c_param_config(I2C_MASTER_NUM, &conf);

    return i2c_driver_install(I2C_MASTER_NUM, conf.mode, I2C_MASTER_RX_BUF_DISABLE, I2C_MASTER_TX_BUF_DISABLE, 0);
}


void app_main(void)
{
    ESP_ERROR_CHECK(i2c_master_init());
    ESP_LOGI(TAG, "I2C initialized successfully");

    // Test Rémi
    const TickType_t xDelay = 1000 / portTICK_PERIOD_MS;
    esp_err_t err;
    char state;

//     vTaskDelay(xDelay);
//     err = motors_read_state(&state);
//     ESP_LOGI(TAG, "state:%i (%s)",state,esp_err_to_name(err));

    vTaskDelay(xDelay);
    err = motors_write_command("o:1,2000;r!");
    ESP_LOGI(TAG, "write (%s)",esp_err_to_name(err));

//     vTaskDelay(xDelay);
//     err = motors_read_state(&state);
//     ESP_LOGI(TAG, "state:%i (%s)",state,esp_err_to_name(err));
}
