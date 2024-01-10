#include "sun_tracker_logic.hpp"

#include "esp_log.h" // replaced by stub/esp_log.h for tests on host

#include "camera.hpp"
#include "image.hpp"

#include <assert.h>

static const char *TAG = "sun_tracker_logic";

static const int LIGHTED_PIXEL_MIN_OFFSET = 100; // Minimal offset from borders min level to consider a pixel "lighted"
static const int MIN_LIGHTED_PIXELS_COUNT = 10;
static const int MIN_SPOT_SIZE_PX = 20;
static const int MAX_DISTANCE_FROM_CENTER_AXIS_PX = 5;
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
    return true;
}

void draw_spot_light_rectangle(CImg<unsigned char> &image, const rectangle_t &spot_light)
{
    image.draw_rectangle(
        spot_light.left_px, spot_light.top_px, spot_light.right_px, spot_light.bottom_px, &WHITE, 1, 0xF0F0F0F0);
    image.draw_rectangle(
        spot_light.left_px, spot_light.top_px, spot_light.right_px, spot_light.bottom_px, &BLACK, 1, 0x0F0F0F0F);
}

void draw_motors_arrow(CImg<unsigned char> &image,
                       int spot_x,
                       int spot_y,
                       bool move_from_left,
                       bool move_from_top,
                       bool move_from_right,
                       bool move_from_bottom)
{

    int arrow_x = 0;
    if (move_from_left) {
        arrow_x = 10;
    } else if (move_from_right) {
        arrow_x = -10;
    }

    int arrow_y = 0;
    if (move_from_top) {
        arrow_y = 10;
    } else if (move_from_bottom) {
        arrow_y = -10;
    }

    image.draw_arrow(spot_x, spot_y, spot_x + arrow_x, spot_y + arrow_y, &BLACK);
}

sun_tracker_logic_result_t sun_tracker_logic_get_best_motors_direction(CImg<unsigned char> &target_img,
                                                                       motors_direction_t &motors_direction)
{
    // assert grayscale image
    assert(target_img.depth() == 1);
    assert(target_img.spectrum() == 1);

    rectangle_t spot_light;
    if (!get_spot_light_rectangle(target_img, spot_light)) {
        return sun_tracker_logic_result_t::NO_SPOT;
    }

    draw_spot_light_rectangle(target_img, spot_light);

    if (spot_light.get_width_px() < MIN_SPOT_SIZE_PX || spot_light.get_height_px() < MIN_SPOT_SIZE_PX) {
        return sun_tracker_logic_result_t::SPOT_TOO_SMALL;
    }

    int spot_x = spot_light.get_center_x_px();
    int spot_y = spot_light.get_center_y_px();
    int target_center_x = target_img.width() / 2;
    int target_center_y = target_img.height() / 2;

    bool move_from_left = (spot_x < target_center_x - MAX_DISTANCE_FROM_CENTER_AXIS_PX);
    bool move_from_top = (spot_y < target_center_y - MAX_DISTANCE_FROM_CENTER_AXIS_PX);
    bool move_from_right = (spot_x > target_center_x + MAX_DISTANCE_FROM_CENTER_AXIS_PX);
    bool move_from_bottom = (spot_y > target_center_y + MAX_DISTANCE_FROM_CENTER_AXIS_PX);

    draw_motors_arrow(target_img, spot_x, spot_y, move_from_left, move_from_top, move_from_right, move_from_bottom);

    if (move_from_left) {
        if (move_from_top) {
            motors_direction = motors_direction_t::DOWN_RIGHT;
        } else if (move_from_bottom) {
            motors_direction = motors_direction_t::UP_RIGHT;
        } else {
            motors_direction = motors_direction_t::RIGHT;
        }
        return sun_tracker_logic_result_t::MUST_MOVE;
    }

    if (move_from_right) {
        if (move_from_top) {
            motors_direction = motors_direction_t::DOWN_LEFT;
        } else if (move_from_bottom) {
            motors_direction = motors_direction_t::UP_LEFT;
        } else {
            motors_direction = motors_direction_t::LEFT;
        }
        return sun_tracker_logic_result_t::MUST_MOVE;
    }

    if (move_from_top) {
        motors_direction = motors_direction_t::DOWN;
        return sun_tracker_logic_result_t::MUST_MOVE;
    }

    if (move_from_bottom) {
        motors_direction = motors_direction_t::UP;
        return sun_tracker_logic_result_t::MUST_MOVE;
    }

    return sun_tracker_logic_result_t::TARGET_REACHED;
}
