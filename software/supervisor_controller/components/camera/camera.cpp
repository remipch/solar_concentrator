#include "camera.hpp"

#include "esp_log.h"
// #include "esp_system.h"

using namespace cimg_library;

static const char *TAG = "camera";

// Hardcoded pinout for CAMERA_MODULE_AI_THINKER
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

#define XCLK_FREQ_HZ 15000000

void camera_init()
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
}

void rgb565_frame_to_rgb888_cimg(camera_fb_t *input, CImg<unsigned char>& output) {
    assert(input->format==PIXFORMAT_RGB565);
    assert(output.width()==input->width);
    assert(output.height()==input->height);
    assert(output.depth()==1);
    assert(output.spectrum()==3);
    int i=0;
    for(int y=0; y<input->height; y++) {
        for(int x=0; x<input->width; x++) {
            // format is deduced from esp-who/components/esp-dl/include/image/dl_image.hpp
            uint16_t rgb565 = ((uint16_t*)input->buf)[i++];
            uint8_t b = (uint8_t)((rgb565 & 0x1F00) >> 5);
            uint8_t g = (uint8_t)(((rgb565 & 0x7) << 5) | ((rgb565 & 0xE000) >> 11));
            uint8_t r = (uint8_t)(rgb565 & 0xF8);
            *output.data(x,y,0,0) = r;
            *output.data(x,y,0,1) = g;
            *output.data(x,y,0,2) = b;
        }
    }
}

void grayscale_frame_to_grayscale_cimg(camera_fb_t *input, CImg<unsigned char>& output) {
    assert(input->format==PIXFORMAT_GRAYSCALE);
    assert(output.width()==input->width);
    assert(output.height()==input->height);
    assert(output.depth()==1);
    assert(output.spectrum()==1);
    // TODO? memcpy (seems to be the same pixel ordering)
    int i=0;
    for(int y=0; y<input->height; y++) {
        for(int x=0; x<input->width; x++) {
            *output.data(x,y,0) = input->buf[i++];
        }
    }
}

// output_buffer must be allocated by the caller
camera_fb_t grayscale_cimg_to_grayscale_frame(CImg<unsigned char>& input, uint8_t *output_buffer) {
    assert(input.depth()==1);
    assert(output_buffer!=NULL);
    camera_fb_t output{
        .buf = output_buffer,
        .len = (size_t)(input.width() * input.height()),
        .width = (size_t)input.width(),
        .height = (size_t)input.height(),
        .format = PIXFORMAT_GRAYSCALE,
        .timestamp = {
            .tv_sec = 0,
            .tv_usec = 0,
        }
    };
    int i=0;
    for(int y=0; y<input.height(); y++) {
        for(int x=0; x<input.width(); x++) {
            uint8_t gray = *input.data(x,y,0);
            output.buf[i++] = gray;
        }
    }
    return output;
}

void rgb888_cimg_to_grayscale_quirc(CImg<unsigned char>& input, uint8_t * output) {
    assert(input.width()==CAMERA_WIDTH);
    assert(input.height()==CAMERA_HEIGHT);
    int i=0;
    for(int y=0; y<input.height(); y++) {
        for(int x=0; x<input.width(); x++) {
            float r = (float)(*input.data(x,y,0,0));
            float g = (float)(*input.data(x,y,0,1));
            float b = (float)(*input.data(x,y,0,2));
            uint8_t gray = (uint8_t)((0.2126 * r) + (0.7152 * g) + (0.0722 * b));
            output[i++] = gray;

            // Temp : change input to grayscale
            *input.data(x,y,0,0) = gray;
            *input.data(x,y,0,1) = gray;
            *input.data(x,y,0,2) = gray;

            if(i<40) {
                ESP_LOGV(TAG, "  x=%u y=%u gray=0x%02x",x,y,gray);
            }
        }
    }
}

// TODO : idem grayscale_cimg_to_grayscale_frame
void rgb888_cimg_to_rgb565_frame(CImg<unsigned char>& input, camera_fb_t *output) {
    assert(input.width()==CAMERA_WIDTH);
    assert(input.height()==CAMERA_HEIGHT);
    output->len = 2 * CAMERA_WIDTH * CAMERA_HEIGHT;
    output->width = CAMERA_WIDTH;
    output->height = CAMERA_HEIGHT;
    output->format = CAMERA_PIXEL_FORMAT;
    output->timestamp.tv_sec = 0;
    output->timestamp.tv_usec = 0;

    int i=0;
    for(int y=0; y<input.height(); y++) {
        for(int x=0; x<input.width(); x++) {
            uint8_t r = *input.data(x,y,0,0);
            uint16_t g = (uint16_t)(*input.data(x,y,0,1));
            uint16_t b = (uint16_t)(*input.data(x,y,0,2));
            uint16_t rgb565 = (r&0xf8) | ((g&0xe0)>>5)| ((g&0x1c)<<11) | ((b&0xf8)<<5);
            ((uint16_t*)output->buf)[i++] = rgb565;
            if(i<40) {
                ESP_LOGV(TAG, "  x=%u y=%u r=0x%02x g=0x%02x b=0x%02x -> rgb565=0x%04x",x,y,r,g,b,rgb565);
            }
        }
    }
}

bool camera_capture(bool drop_current_image, CImg<unsigned char>& grayscale_cimg) {
    assert(grayscale_cimg.width()==CAMERA_WIDTH);
    assert(grayscale_cimg.height()==CAMERA_HEIGHT);
    assert(grayscale_cimg.depth()==1);
    assert(grayscale_cimg.spectrum()==1);

    if(drop_current_image) {
        // 'esp_camera_fb_get' returns the image which is currently captured
        // Drop the currently grabed image to guarantee that a new image is
        // taken after the call to this function
        camera_fb_t *frame = esp_camera_fb_get();
        if(frame==NULL) {
            ESP_LOGE(TAG, "esp_camera_fb_get failed");
            return false;
        }
        esp_camera_fb_return(frame);
    }

    camera_fb_t *frame = esp_camera_fb_get();
    if(frame==NULL) {
        ESP_LOGE(TAG, "esp_camera_fb_get failed");
        return false;
    }

    grayscale_frame_to_grayscale_cimg(frame, grayscale_cimg);

    esp_camera_fb_return(frame);

    return true;
}
