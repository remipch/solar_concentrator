#include "camera.hpp"
#include "image.hpp"
#include "app_wifi.h"
#include "app_httpd.hpp"
#include "motors.hpp"
#include "supervisor.hpp"
#include "target_detector.hpp"
#include "sun_tracker.hpp"
#include "esp_log.h"

extern "C" void app_main()
{
    esp_log_level_set("event", ESP_LOG_INFO);

    app_wifi_main();

    camera_init();
    target_detector_init();
    motors_init();
    sun_tracker_init();
    supervisor_init();

    register_httpd(NULL, NULL, true);
}
