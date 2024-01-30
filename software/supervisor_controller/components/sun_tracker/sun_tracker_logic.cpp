#include "sun_tracker_logic.hpp"

#include "esp_log.h" // replaced by stub/esp_log.h for tests on host

#include "camera.hpp"
#include "image.hpp"
#include "motors_direction.hpp"

#include <assert.h>

static const char *TAG = "sun_tracker_logic";

static const int LIGHTED_PIXEL_MIN_OFFSET = 100; // Minimal offset from borders min level to consider a pixel "lighted"
static const int MIN_LIGHTED_PIXELS_COUNT = 10;
static const int MIN_SPOT_SIZE_PX = 20;
static const int MIN_DISTANCE_FROM_BORDER_PX = 5;
static const int MIN_SPOT_OVERRUN_PX = 5;
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
bool get_spot_light_rectangle(const CImg<unsigned char> &img, rectangle_t &result)
{
    result = {-1, -1, -1, -1};
    unsigned char min_level = get_image_borders_min_level(img) + LIGHTED_PIXEL_MIN_OFFSET;
    ESP_LOGD(TAG, "get_spot_light_rectangle: min_level: %i", min_level);

    cimg_forX(img, x)
    {
        rectangle_t segment = get_vertical_segment(img, x);
        if (count_lighted_pixels(img, segment, min_level) > MIN_LIGHTED_PIXELS_COUNT) {
            result.left_px = x;
            break;
        }
    }
    if (result.left_px < 0) {
        return false;
    }
    cimg_forX(img, x)
    {
        rectangle_t segment = get_vertical_segment(img, img.width() - x - 1);
        if (count_lighted_pixels(img, segment, min_level) > MIN_LIGHTED_PIXELS_COUNT) {
            result.right_px = img.width() - x - 1;
            break;
        }
    }
    if (result.right_px < 0) {
        return false;
    }
    cimg_forY(img, y)
    {
        rectangle_t segment = get_horizontal_segment(img, y);
        if (count_lighted_pixels(img, segment, min_level) > MIN_LIGHTED_PIXELS_COUNT) {
            result.top_px = y;
            break;
        }
    }
    if (result.top_px < 0) {
        return false;
    }
    cimg_forY(img, y)
    {
        rectangle_t segment = get_horizontal_segment(img, img.height() - y - 1);
        if (count_lighted_pixels(img, segment, min_level) > MIN_LIGHTED_PIXELS_COUNT) {
            result.bottom_px = img.height() - y - 1;
            break;
        }
    }
    if (result.bottom_px < 0) {
        return false;
    }

    ESP_LOGD(TAG,
             "get_spot_light_rectangle: left_px: %i ; right_px: %i ; top_px: %i ; bottom_px: %i",
             result.left_px,
             result.right_px,
             result.top_px,
             result.bottom_px);
    return true;
}

void draw_spot_light_rectangle(CImg<unsigned char> &image, const rectangle_t &spot_light)
{
    image.draw_rectangle(
        spot_light.left_px, spot_light.top_px, spot_light.right_px, spot_light.bottom_px, &WHITE, 1, 0xF0F0F0F0);
    image.draw_rectangle(
        spot_light.left_px, spot_light.top_px, spot_light.right_px, spot_light.bottom_px, &BLACK, 1, 0x0F0F0F0F);
}

void draw_motors_arrow(CImg<unsigned char> &image, rectangle_t spot_light, motors_direction_t motors_direction)
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

    int spot_x = spot_light.get_center_x_px();
    int spot_y = spot_light.get_center_y_px();
    image.draw_arrow(spot_x, spot_y, spot_x + arrow_x, spot_y + arrow_y, &BLACK);
}

// To check target_image size does not change between calls
static int target_width = 0;
static int target_height = 0;
rectangle_t spot_light_before_move;

motors_direction_t
get_best_motors_direction(bool move_from_left, bool move_from_top, bool move_from_right, bool move_from_bottom)
{
    if (move_from_left) {
        if (move_from_top) {
            return motors_direction_t::DOWN_RIGHT;
        } else if (move_from_bottom) {
            return motors_direction_t::UP_RIGHT;
        } else {
            return motors_direction_t::RIGHT;
        }
    }
    if (move_from_right) {
        if (move_from_top) {
            return motors_direction_t::DOWN_LEFT;
        } else if (move_from_bottom) {
            return motors_direction_t::UP_LEFT;
        } else {
            return motors_direction_t::LEFT;
        }
    }
    if (move_from_top) {
        return motors_direction_t::DOWN;
    }
    if (move_from_bottom) {
        return motors_direction_t::UP;
    }
    return motors_direction_t::NONE;
}

sun_tracker_logic_result_t sun_tracker_logic_start(CImg<unsigned char> &target_img,
                                                   motors_direction_t &motors_direction)
{
    // assert grayscale image
    assert(target_img.depth() == 1);
    assert(target_img.spectrum() == 1);
    target_width = target_img.width();
    target_height = target_img.height();

    if (!get_spot_light_rectangle(target_img, spot_light_before_move)) {
        ESP_LOGD(TAG, "sun_tracker_logic_start: NO_SPOT_DETECTED");
        return sun_tracker_logic_result_t::NO_SPOT_DETECTED;
    }

    draw_spot_light_rectangle(target_img, spot_light_before_move);

    if (spot_light_before_move.get_width_px() < MIN_SPOT_SIZE_PX
        || spot_light_before_move.get_height_px() < MIN_SPOT_SIZE_PX) {
        ESP_LOGD(TAG, "sun_tracker_logic_start: SPOT_TOO_SMALL");
        return sun_tracker_logic_result_t::SPOT_TOO_SMALL;
    }

    bool move_from_left = (spot_light_before_move.left_px < MIN_DISTANCE_FROM_BORDER_PX);
    bool move_from_top = (spot_light_before_move.top_px < MIN_DISTANCE_FROM_BORDER_PX);
    bool move_from_right = (target_img.width() - 1 - spot_light_before_move.right_px < MIN_DISTANCE_FROM_BORDER_PX);
    bool move_from_bottom = (target_img.width() - 1 - spot_light_before_move.bottom_px < MIN_DISTANCE_FROM_BORDER_PX);

    if ((move_from_left && move_from_right) || (move_from_top && move_from_bottom)) {
        ESP_LOGD(TAG, "sun_tracker_logic_start: SPOT_TOO_BIG");
        return sun_tracker_logic_result_t::SPOT_TOO_BIG;
    }

    motors_direction = get_best_motors_direction(move_from_left, move_from_top, move_from_right, move_from_bottom);
    ESP_LOGD(TAG, "sun_tracker_logic_start: get_best_motors_direction: %s", str(motors_direction));

    draw_motors_arrow(target_img, spot_light_before_move, motors_direction);

    if (motors_direction == motors_direction_t::NONE) {
        ESP_LOGD(TAG, "sun_tracker_logic_start: TARGET_REACHED");
        return sun_tracker_logic_result_t::TARGET_REACHED;
    } else {
        ESP_LOGD(TAG, "sun_tracker_logic_start: MUST_MOVE");
        return sun_tracker_logic_result_t::MUST_MOVE;
    }
}

sun_tracker_logic_result_t sun_tracker_logic_update(CImg<unsigned char> &target_img,
                                                    motors_direction_t &motors_direction)
{
    assert(target_img.depth() == 1);
    assert(target_img.spectrum() == 1);
    assert(target_img.width() == target_width);
    assert(target_img.height() == target_height);

    rectangle_t spot_light_after_move;
    if (!get_spot_light_rectangle(target_img, spot_light_after_move)) {
        ESP_LOGD(TAG, "sun_tracker_logic_update: NO_SPOT_DETECTED");
        return sun_tracker_logic_result_t::NO_SPOT_DETECTED;
    }

    draw_spot_light_rectangle(target_img, spot_light_after_move);

    bool left_overrun = (spot_light_before_move.left_px - spot_light_after_move.left_px > MIN_SPOT_OVERRUN_PX);
    bool up_overrun = (spot_light_before_move.top_px - spot_light_after_move.top_px > MIN_SPOT_OVERRUN_PX);
    bool right_overrun = (spot_light_after_move.right_px - spot_light_before_move.right_px > MIN_SPOT_OVERRUN_PX);
    bool down_overrun = (spot_light_after_move.bottom_px - spot_light_before_move.bottom_px > MIN_SPOT_OVERRUN_PX);

    switch (motors_direction) {
    case motors_direction_t::UP:
        if (up_overrun) {
            motors_direction = motors_direction_t::NONE;
        }
        break;
    case motors_direction_t::UP_RIGHT:
        if (up_overrun || right_overrun) {
            motors_direction = motors_direction_t::NONE;
        }
        break;
    case motors_direction_t::RIGHT:
        if (right_overrun) {
            motors_direction = motors_direction_t::NONE;
        }
        break;
    case motors_direction_t::DOWN_RIGHT:
        if (down_overrun || right_overrun) {
            motors_direction = motors_direction_t::NONE;
        }
        break;
    case motors_direction_t::DOWN:
        if (down_overrun) {
            motors_direction = motors_direction_t::NONE;
        }
        break;
    case motors_direction_t::DOWN_LEFT:
        if (down_overrun || left_overrun) {
            motors_direction = motors_direction_t::NONE;
        }
        break;
    case motors_direction_t::LEFT:
        if (left_overrun) {
            motors_direction = motors_direction_t::NONE;
        }
        break;
    case motors_direction_t::UP_LEFT:
        if (up_overrun || left_overrun) {
            motors_direction = motors_direction_t::NONE;
        }
        break;
    default:
        break;
    }

    draw_motors_arrow(target_img, spot_light_before_move, motors_direction);

    if (motors_direction == motors_direction_t::NONE) {
        ESP_LOGD(TAG, "sun_tracker_logic_update: TARGET_REACHED");
        return sun_tracker_logic_result_t::TARGET_REACHED;
    } else {
        ESP_LOGD(TAG, "sun_tracker_logic_update: MUST_MOVE");
        return sun_tracker_logic_result_t::MUST_MOVE;
    }
}
