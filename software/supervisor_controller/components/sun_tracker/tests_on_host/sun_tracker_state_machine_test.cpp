#include "mini_mock.hpp"

// #include "sun_tracker_callbacks.hpp"
#include "sun_tracker_state_machine.hpp"

#include "motors.hpp"

#include "image.hpp"

// MINI_MOCK_FUNCTION(target_detector_init, void, (), ());
MINI_MOCK_FUNCTION(camera_capture,
                   bool,
                   (bool drop_current_image, CImg<unsigned char> &grayscale_cimg),
                   (drop_current_image, grayscale_cimg));
MINI_MOCK_FUNCTION(target_detector_detect, bool, (CImg<unsigned char> & image, rectangle_t &target), (image, target));

void sun_tracker_result(sun_tracker_result_t result) {}

// sun_tracker image callbacks are for display purpose only, ignore them now in these tests
void dummy_sun_tracker_image(CImg<unsigned char> &cimg) {}

TEST(test_init, []() {
    CImg<unsigned char> dummy_img(CAMERA_WIDTH, CAMERA_HEIGHT, 1, 1);

    // MINI_MOCK_ON_CALL(camera_capture,
    //                   [](bool drop_current_image, CImg<unsigned char> &grayscale_cimg) { return true; });

    sun_tracker_result_t signaled_result;
    sun_tracker_state_t state = sun_tracker_state_machine_update(
        sun_tracker_state_t::UNINITIALIZED,
        sun_tracker_transition_t::NONE,
        [](sun_tracker_result_t result) {
            EXPECT(false); /* should not be called*/
        },
        dummy_sun_tracker_image,
        dummy_sun_tracker_image);

    EXPECT(state == sun_tracker_state_t::IDLE);
});

CREATE_MAIN_ENTRY_POINT();
