
#include "esp_log.h"

#include "image.hpp"
#include "target_detector.hpp"
#include "quirc.h"

#include <climits>
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

template<typename T>
struct quad {
    T top_left;
    T top_right;
    T bottom_left;
    T bottom_right;
};

// Extract ordered corners from capstone
quad<quirc_point> extract_corners(const quirc_capstone *capstone) {
    int min_x = INT_MAX;
    int min_y = INT_MAX;
    int max_x = 0;
    int max_y = 0;
    for(int i=0;i<4;i++) {
        quirc_point corner = capstone->corners[i];
        min_x = std::min(min_x, corner.x);
        min_y = std::min(min_y, corner.y);
        max_x = std::max(max_x, corner.x);
        max_y = std::max(max_y, corner.y);
    }
    return quad<quirc_point> {
        .top_left = {min_x, min_y},
        .top_right = {max_x, min_y},
        .bottom_left = {min_x, max_y},
        .bottom_right = {max_x, max_y},
    };
}

// Each capstones is supposed to be on one of the four corners,
// split them according to their place from average point
quad<const quirc_capstone *>  extract_capstones_quad(
    int capstone_count,
    int average_x,
    int average_y) {

    assert(capstone_count==EXPECTED_CAPSTONE_COUNT);

    quad<const quirc_capstone *> result {
        .top_left = NULL,
        .top_right = NULL,
        .bottom_left = NULL,
        .bottom_right = NULL,
    };

    for(int i=0;i<capstone_count;i++) {
        const quirc_capstone *capstone = quirc_get_capstone(capstone_detector,i);
        if(capstone->center.y < average_y) {
            if(capstone->center.x < average_x) {
                result.top_left = capstone;
            }
            else {
                result.top_right = capstone;
            }
        }
        else {
            if(capstone->center.x < average_x) {
                result.bottom_left = capstone;
            }
            else {
                result.bottom_right = capstone;
            }
        }
    }
    return result;
}

void extract_size(
    const quirc_capstone *capstone,
    int& width, int& height) {

    quad<quirc_point> corners = extract_corners(capstone);
    width = (corners.top_right.x - corners.top_left.x + corners.bottom_right.x - corners.bottom_left.x) / 2;
    height = (corners.bottom_left.y - corners.top_left.y + corners.bottom_right.y - corners.top_right.y) / 2;
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

    int average_x = 0;
    int average_y = 0;
    int average_width = 0;
    int average_height = 0;

    // Draw all detected capstones (for display purpose only)
    for(int i=0;i<capstone_count;i++) {
        const quirc_capstone *capstone = quirc_get_capstone(capstone_detector,i);
        average_x += capstone->center.x;
        average_y += capstone->center.y;

        int capstone_width = 0;
        int capstone_height = 0;
//         quad<quirc_point> corners = extract_corners(capstone);

        extract_size(capstone, capstone_width, capstone_height);
        average_width += capstone_width;
        average_height += capstone_height;

        ESP_LOGV(TAG, "  capstone.center:  %i, %i", capstone->center.x, capstone->center.y);
        ESP_LOGV(TAG, "  capstone.size:  %i, %i", capstone_width, capstone_height);
        for(int i=0;i<4;i++) {
            ESP_LOGV(TAG, "    capstone.corners[%i]: %i , %i\n", i, capstone->corners[i].x, capstone->corners[i].y);
        }
        image.draw_line(capstone->center.x-capstone_width/2, capstone->center.y, capstone->center.x+capstone_width/2, capstone->center.y, &WHITE);
        image.draw_line(capstone->center.x, capstone->center.y-capstone_height/2, capstone->center.x, capstone->center.y+capstone_height/2, &WHITE);
    }

    if(capstone_count!=EXPECTED_CAPSTONE_COUNT) {
        ESP_LOGW(TAG, "Detection failed : %i capstone(s) detected instead of %i ", capstone_count, EXPECTED_CAPSTONE_COUNT);
        return false;
    }

    average_x /= capstone_count;
    average_y /= capstone_count;
    average_width /= capstone_count;
    average_height /= capstone_count;

    quad<const quirc_capstone *> capstones = extract_capstones_quad(capstone_count, average_x, average_y);

    // Here we should have exactly one capstone per corner
    if( capstones.top_left==NULL ||
        capstones.top_right==NULL ||
        capstones.bottom_left==NULL ||
        capstones.bottom_right==NULL) {

        ESP_LOGW(TAG, "Detection failed : incorrect capstones placement");
        return false;
    }

    // Check misalignment < average_size
    if( std::abs(capstones.top_left->center.x - capstones.bottom_left->center.x) > average_width ||
        std::abs(capstones.top_right->center.x - capstones.bottom_right->center.x) > average_width ||
        std::abs(capstones.top_left->center.y - capstones.top_right->center.y) > average_height ||
        std::abs(capstones.bottom_left->center.y - capstones.bottom_right->center.y) > average_height) {

        ESP_LOGW(TAG, "Detection failed : incorrect capstones alignment");
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
