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
    sun_tracker_logic_result_t result = sun_tracker_logic_start(target_img, motors_direction);
    target_img.save("spot_on_center_result.jpg");
    EXPECT(motors_direction == motors_direction_t::NONE);
    EXPECT(result == sun_tracker_logic_result_t::TARGET_REACHED);
});

TEST(typical_move_from_top_right, []() {
    // Start from top right corner : must move down left
    CImg<unsigned char> target_img = load_image_as_grayscale("spot_on_top_right_1.jpg");
    motors_direction_t motors_direction = motors_direction_t::NONE;
    sun_tracker_logic_result_t result = sun_tracker_logic_start(target_img, motors_direction);
    target_img.save("single_panel_typical_move_from_top_right_1.jpg");
    EXPECT(motors_direction == motors_direction_t::DOWN_LEFT);
    EXPECT(result == sun_tracker_logic_result_t::MUST_MOVE);

    // Move not enough to have a spot overrun : must continue to move
    target_img = load_image_as_grayscale("spot_on_top_right_2.jpg");
    result = sun_tracker_logic_update(target_img, motors_direction);
    target_img.save("single_panel_typical_move_from_top_right_2.jpg");
    EXPECT(motors_direction == motors_direction_t::DOWN_LEFT);
    EXPECT(result == sun_tracker_logic_result_t::MUST_MOVE);

    // Move with enough spot overrun
    target_img = load_image_as_grayscale("spot_on_top_right_3.jpg");
    result = sun_tracker_logic_update(target_img, motors_direction);
    target_img.save("single_panel_typical_move_from_top_right_3.jpg");
    EXPECT(motors_direction == motors_direction_t::NONE);
    EXPECT(result == sun_tracker_logic_result_t::TARGET_REACHED);
});

CREATE_MAIN_ENTRY_POINT();
