#include "mini_mock.hpp"

#include "motors_direction.hpp"
#include "sun_tracker_logic.hpp"

CImg<unsigned char> load_image_as_grayscale(const char *image_path)
{
    CImg<unsigned char> image(image_path);
    if (image.spectrum() >= 3) {
        return image.get_RGBtoYCbCr().get_channel(0);
    }
    return image;
}

TEST(when_spot_is_on_center_then_target_reached, []() {
    CImg<unsigned char> target_img = load_image_as_grayscale("spot_on_center.jpg");
    motors_direction_t motors_direction = motors_direction_t::NONE;
    sun_tracker_logic_result_t result = sun_tracker_logic_get_best_motors_direction(target_img, motors_direction);
    target_img.save("spot_on_center_result.jpg");
    EXPECT(motors_direction == motors_direction_t::NONE);
    EXPECT(result == sun_tracker_logic_result_t::TARGET_REACHED);
});

TEST(when_spot_is_on_left_then_move_right, []() {
    CImg<unsigned char> target_img = load_image_as_grayscale("spot_on_left.jpg");
    motors_direction_t motors_direction = motors_direction_t::NONE;
    sun_tracker_logic_result_t result = sun_tracker_logic_get_best_motors_direction(target_img, motors_direction);
    target_img.save("spot_on_left_result.jpg");
    EXPECT(motors_direction == motors_direction_t::RIGHT);
    EXPECT(result == sun_tracker_logic_result_t::MUST_MOVE);
});

TEST(when_spot_is_on_top_right_then_move_down_left, []() {
    CImg<unsigned char> target_img = load_image_as_grayscale("spot_on_top_right.jpg");
    motors_direction_t motors_direction = motors_direction_t::NONE;
    sun_tracker_logic_result_t result = sun_tracker_logic_get_best_motors_direction(target_img, motors_direction);
    target_img.save("spot_on_top_right_result.jpg");
    EXPECT(motors_direction == motors_direction_t::DOWN_LEFT);
    EXPECT(result == sun_tracker_logic_result_t::MUST_MOVE);
});

CREATE_MAIN_ENTRY_POINT();
