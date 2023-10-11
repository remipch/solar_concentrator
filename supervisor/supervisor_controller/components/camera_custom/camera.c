#include "camera.h"

#include "esp_log.h"
#include "esp_system.h"

static const char *TAG = "camera_custom";
static QueueHandle_t xQueueFrameO = NULL;

#if CONFIG_CAMERA_MODULE_AI_THINKER
#define CAMERA_PIN_PWDN 32
#define CAMERA_PIN_RESET -1
#define CAMERA_PIN_XCLK 0
#define CAMERA_PIN_SIOD 26
#define CAMERA_PIN_SIOC 27

#define CAMERA_PIN_D7 35
#define CAMERA_PIN_D6 34
#define CAMERA_PIN_D5 39
#define CAMERA_PIN_D4 36
#define CAMERA_PIN_D3 21
#define CAMERA_PIN_D2 19
#define CAMERA_PIN_D1 18
#define CAMERA_PIN_D0 5
#define CAMERA_PIN_VSYNC 25
#define CAMERA_PIN_HREF 23
#define CAMERA_PIN_PCLK 22
#endif

#define XCLK_FREQ_HZ 15000000


static void task_process_handler(void *arg)
{
    while (true)
    {
        camera_fb_t *frame = esp_camera_fb_get();
        if (frame)
            xQueueSend(xQueueFrameO, &frame, portMAX_DELAY);
    }
}

void register_camera(const QueueHandle_t frame_o)
{
    camera_config_t config;
    config.ledc_channel = LEDC_CHANNEL_0;
    config.ledc_timer = LEDC_TIMER_0;
    config.pin_d0 = CAMERA_PIN_D0;
    config.pin_d1 = CAMERA_PIN_D1;
    config.pin_d2 = CAMERA_PIN_D2;
    config.pin_d3 = CAMERA_PIN_D3;
    config.pin_d4 = CAMERA_PIN_D4;
    config.pin_d5 = CAMERA_PIN_D5;
    config.pin_d6 = CAMERA_PIN_D6;
    config.pin_d7 = CAMERA_PIN_D7;
    config.pin_xclk = CAMERA_PIN_XCLK;
    config.pin_pclk = CAMERA_PIN_PCLK;
    config.pin_vsync = CAMERA_PIN_VSYNC;
    config.pin_href = CAMERA_PIN_HREF;
    config.pin_sccb_sda = CAMERA_PIN_SIOD;
    config.pin_sccb_scl = CAMERA_PIN_SIOC;
    config.pin_pwdn = CAMERA_PIN_PWDN;
    config.pin_reset = CAMERA_PIN_RESET;
    config.xclk_freq_hz = XCLK_FREQ_HZ;
    config.pixel_format = CAMERA_PIXEL_FORMAT;
    config.frame_size = FRAMESIZE_SVGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;
    config.fb_location = CAMERA_FB_IN_PSRAM;
    config.grab_mode = CAMERA_GRAB_LATEST; // (no effect when fb_count==1)

    // camera init
    esp_err_t err = esp_camera_init(&config);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Camera init failed with error 0x%x", err);
        return;
    }

    sensor_t *s = esp_camera_sensor_get();
    s->set_vflip(s, 1); //flip it back

    // Set initial camera config
    // UI will automatically be initialized with these value (see httpd status_handler)

    // Jpeg quality of the http transmitted image
    s->set_quality(s, 63);

    // Gain
    s->set_gain_ctrl(s, false);     // auto gain : not used, always disabled
    s->set_gainceiling(s, (gainceiling_t)0); // ceiling if auto gain is enabled : not used
    s->set_agc_gain(s, 1);  // Gain if auto gain is not enabled

    // Image flip
    s->set_hmirror(s, false);   // hmirror
    s->set_vflip(s, false);     // vflip

    // Expo
    s->set_exposure_ctrl(s, true); // auto expo enabled
    s->set_aec_value(s, 10); // Expo if auto expo is not enabled
    s->set_ae_level(s, 0);   // expo if auto expo is enabled
    s->set_aec2(s, false); // "AEC DSP" no effect

    // Lens light compensation
    s->set_lenc(s, false);

    // Unused color settings (removed from http page)
    s->set_contrast(s, 0);
    s->set_brightness(s, 0);
    s->set_saturation(s, 0);

    // Unused white balance (removed from http page)
    s->set_whitebal(s, false);      // auto white balance : not used
    s->set_awb_gain(s, false);      // not used

    // Other unused options (removed from http page)
    s->set_dcw(s, true);     // must be true (malformed image otherwise)
    s->set_bpc(s, false);      // no effect
    s->set_wpc(s, false);     // no effect
    s->set_colorbar(s, false);      // not used
    s->set_raw_gma(s, false);     // no effect
    s->set_special_effect(s, 0);   // not used
    s->set_wb_mode(s, 0);   // not used

    xQueueFrameO = frame_o;
//     xTaskCreatePinnedToCore(task_process_handler, TAG, 2 * 1024, NULL, 5, NULL, 1);
}
