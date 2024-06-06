#include "sun_tracker_logic.hpp"

#include "esp_log.h" // replaced by stub/esp_log.h for tests on host

#include "camera.hpp"
#include "image.hpp"
#include "motors_direction.hpp"
#include "target_detector.hpp"

#include <assert.h>

static const char *TAG = "sun_tracker_logic";

static const int LIGHTED_PIXEL_MIN_OFFSET = 100; // Minimal offset from borders min level to consider a pixel "lighted"
static const int MIN_LIGHTED_PIXELS_COUNT = 10;
static const int MIN_SPOT_SIZE_PX = 20;
static const int MIN_DISTANCE_FROM_BORDER_PX = 5;
static const int MIN_SPOT_OVERRUN_PX = 10;
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

unsigned char get_image_borders_min_level(const CImg<unsigned char> &img)
{
    unsigned char result = 255;
    cimg_for_borderXY(img, x, y, 1) { result = std::min(result, img(x, y)); }
    return result;
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
    unsigned char min_level = get_image_borders_min_level(target_img) + LIGHTED_PIXEL_MIN_OFFSET;
    ESP_LOGD(TAG, "get_spot_light_rectangle: min_level: %i", min_level);

    cimg_forX(target_img, x)
    {
        rectangle_t segment = get_vertical_segment(target_img, x);
        if (count_lighted_pixels(target_img, segment, min_level) > MIN_LIGHTED_PIXELS_COUNT) {
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
        if (count_lighted_pixels(target_img, segment, min_level) > MIN_LIGHTED_PIXELS_COUNT) {
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
        if (count_lighted_pixels(target_img, segment, min_level) > MIN_LIGHTED_PIXELS_COUNT) {
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
        if (count_lighted_pixels(target_img, segment, min_level) > MIN_LIGHTED_PIXELS_COUNT) {
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

void draw_lighted_borders(CImg<unsigned char> &image, sun_tracker_detection_t detection)
{
    auto target_area = detection.target_area;
    if (detection.left_border) {
        image.draw_line(target_area.left_px - 2,
                        target_area.top_px - 1,
                        target_area.left_px - 2,
                        target_area.bottom_px + 1,
                        &WHITE);
    }
    if (detection.top_border) {
        image.draw_line(
            target_area.left_px - 1, target_area.top_px - 2, target_area.right_px + 1, target_area.top_px - 2, &WHITE);
    }
    if (detection.right_border) {
        image.draw_line(target_area.right_px + 2,
                        target_area.top_px - 1,
                        target_area.right_px + 2,
                        target_area.bottom_px + 1,
                        &WHITE);
    }
    if (detection.bottom_border) {
        image.draw_line(target_area.left_px - 1,
                        target_area.bottom_px + 2,
                        target_area.right_px + 1,
                        target_area.bottom_px + 2,
                        &WHITE);
    }
}

void draw_motors_arrow(
    CImg<unsigned char> &image, rectangle_t spot_light, int offset_x, int offset_y, motors_direction_t motors_direction)
{
    int arrow_x = 0;
    int arrow_y = 0;

    switch (motors_direction) {
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

    int spot_x = spot_light.get_center_x_px() + offset_x;
    int spot_y = spot_light.get_center_y_px() + offset_y;
    image.draw_arrow(spot_x, spot_y, spot_x + arrow_x, spot_y + arrow_y, &BLACK);
}

// Find the best motors direction to move away from lighted borders
motors_direction_t get_best_motors_direction(sun_tracker_detection_t detection)
{
    if (detection.left_border) {
        if (detection.top_border) {
            return motors_direction_t::DOWN_RIGHT;
        } else if (detection.bottom_border) {
            return motors_direction_t::UP_RIGHT;
        } else {
            return motors_direction_t::RIGHT;
        }
    }
    if (detection.right_border) {
        if (detection.top_border) {
            return motors_direction_t::DOWN_LEFT;
        } else if (detection.bottom_border) {
            return motors_direction_t::UP_LEFT;
        } else {
            return motors_direction_t::LEFT;
        }
    }
    if (detection.top_border) {
        return motors_direction_t::DOWN;
    }
    if (detection.bottom_border) {
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
        .left_border = false,
        .top_border = false,
        .right_border = false,
        .bottom_border = false,
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

    int target_width = detection.target_area.get_width_px();
    int target_height = detection.target_area.get_height_px();
    detection.left_border = (detection.spot_light.left_px < MIN_DISTANCE_FROM_BORDER_PX);
    detection.top_border = (detection.spot_light.top_px < MIN_DISTANCE_FROM_BORDER_PX);
    detection.right_border = (target_width - 1 - detection.spot_light.right_px < MIN_DISTANCE_FROM_BORDER_PX);
    detection.bottom_border = (target_height - 1 - detection.spot_light.bottom_px < MIN_DISTANCE_FROM_BORDER_PX);

    draw_lighted_borders(full_img, detection);

    if ((detection.left_border && detection.right_border) || (detection.top_border && detection.bottom_border)) {
        detection.result = sun_tracker_detection_result_t::SPOT_TOO_BIG;
        ESP_LOGW(TAG, "sun_tracker_logic_detect: SPOT_TOO_BIG");
        return detection;
    }

    detection.result = sun_tracker_detection_result_t::SUCCESS;
    detection.direction = get_best_motors_direction(detection);
    ESP_LOGD(TAG, "sun_tracker_logic_detect: SUCCESS (direction: %s)", str(detection.direction));

    return detection;
}

motors_direction_t sun_tracker_logic_update(sun_tracker_detection_t detection_before_move,
                                            sun_tracker_detection_t detection_after_move)
{

    bool left_overrun =
        (detection_before_move.spot_light.left_px - detection_after_move.spot_light.left_px > MIN_SPOT_OVERRUN_PX);
    bool up_overrun =
        (detection_before_move.spot_light.top_px - detection_after_move.spot_light.top_px > MIN_SPOT_OVERRUN_PX);
    bool right_overrun =
        (detection_after_move.spot_light.right_px - detection_before_move.spot_light.right_px > MIN_SPOT_OVERRUN_PX);
    bool down_overrun =
        (detection_after_move.spot_light.bottom_px - detection_before_move.spot_light.bottom_px > MIN_SPOT_OVERRUN_PX);

    ESP_LOGD(TAG,
             "sun_tracker_logic_update: overruns l:%i u:%i r:%i b:%i",
             left_overrun,
             up_overrun,
             right_overrun,
             down_overrun);

    switch (detection_before_move.direction) {
    case motors_direction_t::UP:
        if (up_overrun) {
            return motors_direction_t::NONE;
        }
        break;
    case motors_direction_t::UP_RIGHT:
        if (up_overrun || right_overrun) {
            return motors_direction_t::NONE;
        }
        break;
    case motors_direction_t::RIGHT:
        if (right_overrun) {
            return motors_direction_t::NONE;
        }
        break;
    case motors_direction_t::DOWN_RIGHT:
        if (down_overrun || right_overrun) {
            return motors_direction_t::NONE;
        }
        break;
    case motors_direction_t::DOWN:
        if (down_overrun) {
            return motors_direction_t::NONE;
        }
        break;
    case motors_direction_t::DOWN_LEFT:
        if (down_overrun || left_overrun) {
            return motors_direction_t::NONE;
        }
        break;
    case motors_direction_t::LEFT:
        if (left_overrun) {
            return motors_direction_t::NONE;
        }
        break;
    case motors_direction_t::UP_LEFT:
        if (up_overrun || left_overrun) {
            return motors_direction_t::NONE;
        }
        break;
    default:
        break;
    }

    // Continue to move
    ESP_LOGD(TAG, "sun_tracker_logic_update: move (direction: %s)", str(detection_after_move.direction));
    return detection_after_move.direction;
}
