
#include "esp_log.h"

#include "image.hpp"

#include "camera.hpp"   // use camera size at compile time to save memory allocations

#include "quirc.h"

static const char *TAG = "target_detector";

static struct quirc *capstone_detector;

// Working image, defined static to save future allocations
// static CImg<unsigned char> img(CAMERA_WIDTH, CAMERA_HEIGHT, 1, 3);

void target_detector_init() {

    capstone_detector = quirc_new();

    if (quirc_resize(capstone_detector, 800, 600) < 0)
		ESP_LOGE(TAG, "quirc_resize fails");
}

#if 0
void draw_something(camera_fb_t *frame) {
    ESP_LOGD(TAG, "draw_something(%u * %u)",(int)frame->width,(int)frame->height);
    assert(frame->width==CAMERA_WIDTH);
    assert(frame->height==CAMERA_HEIGHT);

    // Create working image (allocate new buffer)

    rgb565ToRgb888(frame, img);

    // Draw something using CImg lib
    ESP_LOGD(TAG, "Draw");
    const unsigned char blue[] = { 0,0,128 };
    img.draw_line(5,0,5,70,blue).draw_ellipse(120,170,20,30,0,blue);
    const unsigned char green[] = { 0,255,0 };
    img.draw_line(4,0,4,70,green).draw_ellipse(110,160,20,30,0,green);
    const unsigned char red[] = { 255,0,0 };
    img.draw_line(3,0,3,70,red).draw_ellipse(100,150,20,30,0,red);
    const unsigned char white[] = { 255,255,255 };
    img.draw_text(10,10,"Hello",white,0,1,16);

    ESP_LOGD(TAG, "Convert back to rgb565");
    rgb888ToRgb565(img, frame);
}

void detect_target(camera_fb_t *frame) {
    ESP_LOGD(TAG, "detect_target(%u * %u)",(int)frame->width,(int)frame->height);
    assert(frame->width==CAMERA_WIDTH);
    assert(frame->height==CAMERA_HEIGHT);

    ESP_LOGD(TAG, "Convert from frame_rgb565(c,x,y) to CImg_gray8(x,y,z,c)");
    rgb565ToRgb888(frame, img);

    uint8_t * quirc_image = quirc_begin(capstone_detector, NULL, NULL);
    rgb888ToQuircGrayscale(img, quirc_image);
    quirc_end(capstone_detector);

    int capstone_count = quirc_capstone_count(capstone_detector);
    ESP_LOGI(TAG, "%i capstone(s) detected", capstone_count);
    if(capstone_count>0) {
        const unsigned char red[] = { 255,0,0 };

        for(int i=0;i<capstone_count;i++) {
            printf("  capstone[%i]:\n",i);
            const struct quirc_capstone *capstone = quirc_get_capstone(capstone_detector,i);
            ESP_LOGV(TAG, "    capstone.center:  %i, %i", capstone->center.x, capstone->center.y);
            for(int i=0;i<4;i++) {
                ESP_LOGV(TAG, "    capstone.corners[%i]: %i , %i\n", i, capstone->corners[i].x, capstone->corners[i].y);
            }
            img.draw_line(capstone->center.x-20, capstone->center.y, capstone->center.x+20, capstone->center.y, red);
            img.draw_line(capstone->center.x, capstone->center.y-20, capstone->center.x, capstone->center.y+20, red);
        }
    }

    ESP_LOGD(TAG, "Convert back to rgb565");
    rgb888ToRgb565(img, frame);
}
#endif
