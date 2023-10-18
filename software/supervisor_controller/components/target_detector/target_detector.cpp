
#include "esp_log.h"

#include "image.hpp"
#include "target_detector.hpp"
#include "quirc.h"

#include <assert.h>

static const char *TAG = "target_detector";

static const int EXPECTED_CAPSTONE_COUNT = 4;
// static const int MAX_VERTICAL_MISALIGNMENT_PX = 10;
// static const int MAX_HORIZONTAL_MISALIGNMENT_PX = 10;
// static const int MIN_AREA_SIZE_PX = 10;
static const unsigned char WHITE = 255;

static struct quirc *capstone_detector;

void target_detector_init() {
    capstone_detector = quirc_new();
}

bool target_detector_detect(CImg<unsigned char>& image, rectangle_t& target) {
    // assert grayscale image
    assert(image.depth()==1);
    assert(image.spectrum()==1);

    // TODO : resize only if dimensions have changed
    // TODO : make quirc working on a given buffer without copy, without preallocation, without dimension passing
    assert(quirc_resize(capstone_detector, image.width(), image.height()) == 0);

    uint8_t * quirc_image = quirc_begin(capstone_detector, NULL, NULL);
    memcpy(quirc_image, image.data(), image.width() * image.height());
    quirc_end(capstone_detector);

    int capstone_count = quirc_capstone_count(capstone_detector);

    // Draw all detected capstone (for display purpose only)
    for(int i=0;i<capstone_count;i++) {
        printf("  capstone[%i]:\n",i);
        const struct quirc_capstone *capstone = quirc_get_capstone(capstone_detector,i);
        ESP_LOGV(TAG, "    capstone.center:  %i, %i", capstone->center.x, capstone->center.y);
        for(int i=0;i<4;i++) {
            ESP_LOGV(TAG, "    capstone.corners[%i]: %i , %i\n", i, capstone->corners[i].x, capstone->corners[i].y);
        }
        image.draw_line(capstone->center.x-20, capstone->center.y, capstone->center.x+20, capstone->center.y, &WHITE);
        image.draw_line(capstone->center.x, capstone->center.y-20, capstone->center.x, capstone->center.y+20, &WHITE);
    }

    if(capstone_count!=EXPECTED_CAPSTONE_COUNT) {
        ESP_LOGW(TAG, "Detection failed : %i capstone(s) detected instead of %i ", capstone_count, EXPECTED_CAPSTONE_COUNT);
        return false;
    }
    return true;
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
#endif
