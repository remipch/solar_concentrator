#include "mini_mock.hpp"

#include "motors_direction.hpp"
#include "sun_tracker_logic.hpp"

MINI_MOCK_FUNCTION(target_detector_detect, bool, (CImg<unsigned char> & image, rectangle_t &target), (image, target));

CImg<unsigned char> load_image_as_grayscale(const char *image_path)
{
    CImg<unsigned char> image(image_path);
    if (image.spectrum() >= 3) {
        return image.get_RGBtoYCbCr().get_channel(0);
    }
    return image;
}

TEST(detect_spot_on_center, []() {
    MINI_MOCK_ON_CALL(target_detector_detect, [](CImg<unsigned char> &image, rectangle_t &target) {
        target = {120, 195, 200, 250};
        return true;
    });

    CImg<unsigned char> full_img = load_image_as_grayscale("spot_on_center.jpg");

    sun_tracker_detection_t detection = sun_tracker_logic_detect(full_img);

    full_img.save("detect_spot_on_center_result.jpg");
    EXPECT(detection.result == sun_tracker_detection_result_t::SUCCESS);
    EXPECT(detection.spot_light.left_px == 20);
    EXPECT(detection.spot_light.top_px == 10);
    EXPECT(detection.spot_light.right_px == 58);
    EXPECT(detection.spot_light.bottom_px == 46);
    EXPECT(!detection.left_border);
    EXPECT(!detection.top_border);
    EXPECT(!detection.right_border);
    EXPECT(!detection.bottom_border);
    EXPECT(detection.direction == motors_direction_t::NONE);
});

TEST(detect_spot_on_left_border, []() {
    MINI_MOCK_ON_CALL(target_detector_detect, [](CImg<unsigned char> &image, rectangle_t &target) {
        target = {140, 195, 200, 250};
        return true;
    });

    CImg<unsigned char> full_img = load_image_as_grayscale("spot_on_center.jpg");

    sun_tracker_detection_t detection = sun_tracker_logic_detect(full_img);

    full_img.save("detect_spot_on_left_border_result.jpg");
    EXPECT(detection.result == sun_tracker_detection_result_t::SUCCESS);
    EXPECT(detection.spot_light.left_px == 0);
    EXPECT(detection.spot_light.top_px == 10);
    EXPECT(detection.spot_light.right_px == 38);
    EXPECT(detection.spot_light.bottom_px == 46);
    EXPECT(detection.left_border);
    EXPECT(!detection.top_border);
    EXPECT(!detection.right_border);
    EXPECT(!detection.bottom_border);
    EXPECT(detection.direction == motors_direction_t::RIGHT);
});

TEST(detect_spot_to_small, []() {
    MINI_MOCK_ON_CALL(target_detector_detect, [](CImg<unsigned char> &image, rectangle_t &target) {
        target = {140, 195, 200, 250};
        return true;
    });

    CImg<unsigned char> full_img = load_image_as_grayscale("small_spot.jpg");

    sun_tracker_detection_t detection = sun_tracker_logic_detect(full_img);

    full_img.save("detect_spot_to_small_result.jpg");
    EXPECT(detection.result == sun_tracker_detection_result_t::SPOT_TOO_SMALL);
});

TEST(detect_spot_to_big, []() {
    MINI_MOCK_ON_CALL(target_detector_detect, [](CImg<unsigned char> &image, rectangle_t &target) {
        target = {125, 200, 200, 245};
        return true;
    });

    CImg<unsigned char> full_img = load_image_as_grayscale("spot_on_center.jpg");

    sun_tracker_detection_t detection = sun_tracker_logic_detect(full_img);

    full_img.save("detect_spot_to_big_result.jpg");
    EXPECT(detection.result == sun_tracker_detection_result_t::SPOT_TOO_BIG);
});

TEST(typical_move_from_top_right, []() {
    sun_tracker_detection_t detection_before_move{
        .result = sun_tracker_detection_result_t::SUCCESS,
        .target_area = {100, 200, 200, 250},
        .spot_light = {65, 5, 95, 35},
        .left_border = false,
        .top_border = true,
        .right_border = true,
        .bottom_border = false,
        .direction = motors_direction_t::DOWN_LEFT,
    };

    // Move not enough to have a spot overrun : must continue to move
    sun_tracker_detection_t detection_after_move_1{
        .result = sun_tracker_detection_result_t::SUCCESS,
        .target_area = {100, 200, 200, 250},
        .spot_light = {62, 5, 92, 35},
        .left_border = false,
        .top_border = true,
        .right_border = true,
        .bottom_border = false,
        .direction = motors_direction_t::DOWN_LEFT,
    };
    motors_direction_t motors_direction = sun_tracker_logic_update(detection_before_move, detection_after_move_1);
    EXPECT(motors_direction == motors_direction_t::DOWN_LEFT);

    // Move with enough spot overrun
    sun_tracker_detection_t detection_after_move_2{
        .result = sun_tracker_detection_result_t::SUCCESS,
        .target_area = {100, 200, 200, 250},
        .spot_light = {55, 5, 85, 35},
        .left_border = false,
        .top_border = true,
        .right_border = false,
        .bottom_border = false,
        .direction = motors_direction_t::DOWN,
    };
    motors_direction = sun_tracker_logic_update(detection_before_move, detection_after_move_2);
    EXPECT(motors_direction == motors_direction_t::NONE);
});

CREATE_MAIN_ENTRY_POINT();
