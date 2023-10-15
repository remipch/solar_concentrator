#include "camera.h"
#include "image_processing.hpp"
#include "app_wifi.h"
#include "app_httpd.hpp"
#include "motors.hpp"
#include "supervisor.hpp"
#include "esp_log.h"

static QueueHandle_t xQueueAIFrame = NULL;
static QueueHandle_t xQueueHttpFrame = NULL;

extern "C" void app_main()
{
    esp_log_level_set("event", ESP_LOG_INFO);

    app_wifi_main();
    motors_init();
    supervisor_init();

    xQueueAIFrame = xQueueCreate(1, sizeof(camera_fb_t *));
    xQueueHttpFrame = xQueueCreate(1, sizeof(camera_fb_t *));

    register_camera(xQueueAIFrame);
    register_image_processing(xQueueAIFrame, NULL, NULL, xQueueHttpFrame);
    register_httpd(xQueueHttpFrame, NULL, true);
}

