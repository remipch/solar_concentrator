#include "camera.h"

#include "esp_log.h"
#include "esp_system.h"

static const char *TAG = "who_camera";
static QueueHandle_t xQueueFrameO = NULL;

static void task_process_handler(void *arg)
{
    while (true)
    {
        camera_fb_t *frame = esp_camera_fb_get();
        if (frame)
            xQueueSend(xQueueFrameO, &frame, portMAX_DELAY);
    }
}

void register_camera(const pixformat_t pixel_fromat,
                     const framesize_t frame_size,
                     const uint8_t fb_count,
                     const QueueHandle_t frame_o)
{
    ESP_LOGI(TAG, "Camera module is %s", CAMERA_MODULE_NAME);

#if CONFIG_CAMERA_MODULE_ESP_EYE || CONFIG_CAMERA_MODULE_ESP32_CAM_BOARD
    /* IO13, IO14 is designed for JTAG by default,
     * to use it as generalized input,
     * firstly declair it as pullup input */
    gpio_config_t conf;
    conf.mode = GPIO_MODE_INPUT;
    conf.pull_up_en = GPIO_PULLUP_ENABLE;
    conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    conf.intr_type = GPIO_INTR_DISABLE;
    conf.pin_bit_mask = 1LL << 13;
    gpio_config(&conf);
    conf.pin_bit_mask = 1LL << 14;
    gpio_config(&conf);
#endif

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
    config.pixel_format = pixel_fromat;
    config.frame_size = frame_size;
    config.jpeg_quality = 12;
    config.fb_count = fb_count;
    config.fb_location = CAMERA_FB_IN_PSRAM;
    config.grab_mode = CAMERA_GRAB_LATEST;

    // camera init
    esp_err_t err = esp_camera_init(&config);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Camera init failed with error 0x%x", err);
        return;
    }

    sensor_t *s = esp_camera_sensor_get();
    s->set_vflip(s, 1); //flip it back
    //initial sensors are flipped vertically and colors are a bit saturated
    if (s->id.PID == OV3660_PID)
    {
        s->set_brightness(s, 1);  //up the blightness just a bit
        s->set_saturation(s, -2); //lower the saturation
    }

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
