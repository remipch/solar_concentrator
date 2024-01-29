#include "app_httpd.hpp"
#include "app_wifi.h"
#include "camera.hpp"
#include "esp_log.h"
#include "image.hpp"
#include "motors.hpp"
#include "sun_tracker.hpp"
#include "supervisor.hpp"
#include "target_detector.hpp"

extern "C" void app_main()
{
    esp_log_level_set("event", ESP_LOG_INFO);
    esp_log_level_set("httpd_parse", ESP_LOG_INFO);
    esp_log_level_set("httpd_txrx", ESP_LOG_INFO);
    esp_log_level_set("httpd_uri", ESP_LOG_INFO);
    esp_log_level_set("httpd_sess", ESP_LOG_INFO);
    esp_log_level_set("httpd", ESP_LOG_INFO);
    esp_log_level_set("wifi", ESP_LOG_INFO);

    esp_log_level_set("target_detector", ESP_LOG_INFO);

    app_wifi_main();

    camera_init();
    target_detector_init();
    motors_init();
    sun_tracker_init();
    supervisor_init();

    register_httpd(NULL, NULL, true);
}
