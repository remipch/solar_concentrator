// Copyright (C) 2024 Rémi Peuchot
// This code is distributed under GNU GPL v3 license

#include "sun_tracker_logic.hpp"

#include "esp_log.h" // replaced by stub/esp_log.h for tests on host

#include "camera.hpp"
#include "image.hpp"
#include "motors_direction.hpp"
#include "target_detector.hpp"

#include <assert.h>

static const char *TAG = "sun_tracker_logic";

static const int MIN_LIGHTED_PIXEL_LEVEL = 250;
static const int MIN_LIGHTED_PIXELS_COUNT = 10;
static const int MIN_SPOT_SIZE_PX = 20;
static const int MAX_DISTANCE_FROM_TARGET_CENTER_PX = 5;
static const unsigned char BLACK = 0;
static const unsigned char WHITE = 255;

rectangle_t get_vertical_segment(const CImg<unsigned char> &img, int x)
{
    assert(x >= 0);
    assert(x < img.width());
    return {
        .left_px = x,
        .top_px = 0,
        .right_px = x,
        .bottom_px = img.height() - 1,
    };
}

rectangle_t get_horizontal_segment(const CImg<unsigned char> &img, int y)
{
    assert(y >= 0);
    assert(y < img.height());
    return {
        .left_px = 0,
        .top_px = y,
        .right_px = img.width() - 1,
        .bottom_px = y,
    };
}

int count_lighted_pixels(const CImg<unsigned char> &img, rectangle_t rect, unsigned char min_level)
{
    int result = 0;
    for (int x = rect.left_px; x <= rect.right_px; x++) {
        for (int y = rect.top_px; y <= rect.bottom_px; y++) {
            unsigned char v = img(x, y);
            if (v >= min_level) {
                result++;
            }
        }
    }
    return result;
}

// return true if spot has been found
bool get_spot_light_rectangle(const CImg<unsigned char> &full_img,
                              rectangle_t &target_area,
                              rectangle_t &result_in_target_area)
{
    // TODO? work directly in full_img
    CImg<unsigned char> target_img =
        full_img.get_crop(target_area.left_px, target_area.top_px, target_area.right_px, target_area.bottom_px);

    result_in_target_area = {-1, -1, -1, -1};

    cimg_forX(target_img, x)
    {
        rectangle_t segment = get_vertical_segment(target_img, x);
        if (count_lighted_pixels(target_img, segment, MIN_LIGHTED_PIXEL_LEVEL) > MIN_LIGHTED_PIXELS_COUNT) {
            result_in_target_area.left_px = x;
            break;
        }
    }
    if (result_in_target_area.left_px < 0) {
        return false;
    }
    cimg_forX(target_img, x)
    {
        rectangle_t segment = get_vertical_segment(target_img, target_img.width() - x - 1);
        if (count_lighted_pixels(target_img, segment, MIN_LIGHTED_PIXEL_LEVEL) > MIN_LIGHTED_PIXELS_COUNT) {
            result_in_target_area.right_px = target_img.width() - x - 1;
            break;
        }
    }
    if (result_in_target_area.right_px < 0) {
        return false;
    }
    cimg_forY(target_img, y)
    {
        rectangle_t segment = get_horizontal_segment(target_img, y);
        if (count_lighted_pixels(target_img, segment, MIN_LIGHTED_PIXEL_LEVEL) > MIN_LIGHTED_PIXELS_COUNT) {
            result_in_target_area.top_px = y;
            break;
        }
    }
    if (result_in_target_area.top_px < 0) {
        return false;
    }
    cimg_forY(target_img, y)
    {
        rectangle_t segment = get_horizontal_segment(target_img, target_img.height() - y - 1);
        if (count_lighted_pixels(target_img, segment, MIN_LIGHTED_PIXEL_LEVEL) > MIN_LIGHTED_PIXELS_COUNT) {
            result_in_target_area.bottom_px = target_img.height() - y - 1;
            break;
        }
    }
    if (result_in_target_area.bottom_px < 0) {
        return false;
    }

    ESP_LOGD(TAG,
             "get_spot_light_rectangle: left_px: %i ; top_px: %i ; right_px: %i ; bottom_px: %i",
             result_in_target_area.left_px,
             result_in_target_area.top_px,
             result_in_target_area.right_px,
             result_in_target_area.bottom_px);
    return true;
}

void draw_spot_light_rectangle(CImg<unsigned char> &image, sun_tracker_detection_t detection)
{
    image.draw_rectangle(detection.spot_light.left_px + detection.target_area.left_px,
                         detection.spot_light.top_px + detection.target_area.top_px,
                         detection.spot_light.right_px + detection.target_area.left_px,
                         detection.spot_light.bottom_px + detection.target_area.top_px,
                         &WHITE,
                         1,
                         0xF0F0F0F0);
    image.draw_rectangle(detection.spot_light.left_px + detection.target_area.left_px,
                         detection.spot_light.top_px + detection.target_area.top_px,
                         detection.spot_light.right_px + detection.target_area.left_px,
                         detection.spot_light.bottom_px + detection.target_area.top_px,
                         &BLACK,
                         1,
                         0x0F0F0F0F);
}

void draw_motors_arrow(CImg<unsigned char> &image, sun_tracker_detection_t detection)
{
    int arrow_x = 0;
    int arrow_y = 0;

    switch (detection.direction) {
    case motors_direction_t::UP:
        arrow_y = -10;
        break;
    case motors_direction_t::UP_RIGHT:
        arrow_x = 10;
        arrow_y = -10;
        break;
    case motors_direction_t::RIGHT:
        arrow_x = 10;
        break;
    case motors_direction_t::DOWN_RIGHT:
        arrow_x = 10;
        arrow_y = 10;
        break;
    case motors_direction_t::DOWN:
        arrow_y = 10;
        break;
    case motors_direction_t::DOWN_LEFT:
        arrow_x = -10;
        arrow_y = 10;
        break;
    case motors_direction_t::LEFT:
        arrow_x = -10;
        break;
    case motors_direction_t::UP_LEFT:
        arrow_x = -10;
        arrow_y = -10;
        break;
    default:
        break;
    }

    int spot_x = detection.spot_light.get_center_x_px() + detection.target_area.left_px;
    int spot_y = detection.spot_light.get_center_y_px() + detection.target_area.top_px;
    image.draw_arrow(spot_x, spot_y, spot_x + arrow_x, spot_y + arrow_y, &BLACK, 1, 45, -20);
}

// Find the best motors direction to move away from lighted borders
motors_direction_t get_best_motors_direction(sun_tracker_detection_t detection)
{
    int target_center_x = detection.target_area.get_width_px() / 2;
    int target_center_y = detection.target_area.get_height_px() / 2;
    int spot_light_center_x = detection.spot_light.get_center_x_px();
    int spot_light_center_y = detection.spot_light.get_center_y_px();
    bool go_up = (spot_light_center_y > target_center_y + MAX_DISTANCE_FROM_TARGET_CENTER_PX);
    bool go_down = (spot_light_center_y < target_center_y - MAX_DISTANCE_FROM_TARGET_CENTER_PX);
    if (spot_light_center_x > target_center_x + MAX_DISTANCE_FROM_TARGET_CENTER_PX) {
        if (go_up) {
            return motors_direction_t::UP_LEFT;
        } else if (go_down) {
            return motors_direction_t::DOWN_LEFT;
        } else {
            return motors_direction_t::LEFT;
        }
    }
    if (spot_light_center_x < target_center_x - MAX_DISTANCE_FROM_TARGET_CENTER_PX) {
        if (go_up) {
            return motors_direction_t::UP_RIGHT;
        } else if (go_down) {
            return motors_direction_t::DOWN_RIGHT;
        } else {
            return motors_direction_t::RIGHT;
        }
    }
    if (go_down) {
        return motors_direction_t::DOWN;
    }
    if (go_up) {
        return motors_direction_t::UP;
    }
    return motors_direction_t::NONE;
}

sun_tracker_detection_t sun_tracker_logic_detect(CImg<unsigned char> &full_img)
{
    sun_tracker_detection_t detection{
        .result = sun_tracker_detection_result_t::UNKNOWN,
        .target_area = {-1, -1, -1, -1},
        .spot_light = {-1, -1, -1, -1},
        .direction = motors_direction_t::NONE,
    };

    if (!target_detector_detect(full_img, detection.target_area)) {
        detection.result = sun_tracker_detection_result_t::TARGET_NOT_DETECTED;
        ESP_LOGW(TAG, "sun_tracker_logic_detect: TARGET_NOT_DETECTED");
        return detection;
    }

    if (!get_spot_light_rectangle(full_img, detection.target_area, detection.spot_light)) {
        detection.result = sun_tracker_detection_result_t::SPOT_NOT_DETECTED;
        ESP_LOGW(TAG, "sun_tracker_logic_detect: SPOT_NOT_DETECTED");
        return detection;
    }

    draw_spot_light_rectangle(full_img, detection);

    if (detection.spot_light.get_width_px() < MIN_SPOT_SIZE_PX
        || detection.spot_light.get_height_px() < MIN_SPOT_SIZE_PX) {
        detection.result = sun_tracker_detection_result_t::SPOT_TOO_SMALL;
        ESP_LOGW(TAG, "sun_tracker_logic_detect: SPOT_TOO_SMALL");
        return detection;
    }

    detection.result = sun_tracker_detection_result_t::SUCCESS;
    detection.direction = get_best_motors_direction(detection);
    ESP_LOGD(TAG, "sun_tracker_logic_detect: SUCCESS (direction: %s)", str(detection.direction));

    draw_motors_arrow(full_img, detection);

    return detection;
}
