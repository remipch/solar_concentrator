#include "mini_mock.hpp"

#include "image.hpp"
#include "motors.hpp"
#include "sun_tracker_logic.hpp"
#include "sun_tracker_state_machine.hpp"

MINI_MOCK_FUNCTION(camera_capture,
                   bool,
                   (bool drop_current_image, CImg<unsigned char> &grayscale_cimg),
                   (drop_current_image, grayscale_cimg));
MINI_MOCK_FUNCTION(sun_tracker_logic_detect, sun_tracker_detection_t, (CImg<unsigned char> & full_img), (full_img));
MINI_MOCK_FUNCTION(motors_start_move_one_step, void, (motors_direction_t direction), (direction));

// sun_tracker image callbacks are for display purpose only,
void drop(CImg<unsigned char> &img) {}

TEST(typical_scenario, []() {
    CImg<unsigned char> img;
    sun_tracker_result_t result;

    // Define common dummy mocks used for all tests bellow
    MINI_MOCK_ON_CALL(
        camera_capture,
        [](bool drop_current_image, CImg<unsigned char> &img) {
            // return dummy black image (not used because sun_tracker_logic is mocked)
            img = CImg<unsigned char>(CAMERA_WIDTH, CAMERA_HEIGHT, 1, 1, 0);
            return true;
        },
        7);

    // From 'UNINITIALIZED' state
    sun_tracker_state_t state = sun_tracker_state_machine_update(
        sun_tracker_state_t::UNINITIALIZED, sun_tracker_transition_t::NONE, drop, result);
    EXPECT(result == sun_tracker_result_t::UNKNOWN);
    EXPECT(state == sun_tracker_state_t::IDLE);

    // From 'IDLE' state without transition, when detection return an error
    MINI_MOCK_ON_CALL(sun_tracker_logic_detect, [](CImg<unsigned char> &image) {
        return sun_tracker_detection_t{.result = sun_tracker_detection_result_t::SPOT_NOT_DETECTED};
    });
    state = sun_tracker_state_machine_update(sun_tracker_state_t::IDLE, sun_tracker_transition_t::NONE, drop, result);
    EXPECT(result == sun_tracker_result_t::ERROR);
    EXPECT(state == sun_tracker_state_t::IDLE);

    // From 'IDLE' with 'START' transition, when detection return an error
    MINI_MOCK_ON_CALL(sun_tracker_logic_detect, [](CImg<unsigned char> &image) {
        return sun_tracker_detection_t{.result = sun_tracker_detection_result_t::SPOT_NOT_DETECTED};
    });
    state = sun_tracker_state_machine_update(sun_tracker_state_t::IDLE, sun_tracker_transition_t::START, drop, result);
    EXPECT(result == sun_tracker_result_t::ERROR);
    EXPECT(state == sun_tracker_state_t::IDLE);

    // From 'IDLE' state with 'START' transition, when detection return 'SUCCESS' without motors move
    MINI_MOCK_ON_CALL(sun_tracker_logic_detect, [](CImg<unsigned char> &image) {
        return sun_tracker_detection_t{
            .result = sun_tracker_detection_result_t::SUCCESS,
            .target_area = {100, 200, 300, 400},
            .spot_light = {65, 5, 95, 35},
            .direction = motors_direction_t::NONE,
        };
    });
    state = sun_tracker_state_machine_update(sun_tracker_state_t::IDLE, sun_tracker_transition_t::START, drop, result);
    EXPECT(result == sun_tracker_result_t::SUCCESS);
    EXPECT(state == sun_tracker_state_t::IDLE);

    // From 'IDLE' state with 'START' transition, when detection return 'SUCCESS' with motors move
    MINI_MOCK_ON_CALL(sun_tracker_logic_detect, [](CImg<unsigned char> &image) {
        return sun_tracker_detection_t{
            .result = sun_tracker_detection_result_t::SUCCESS,
            .target_area = {100, 200, 300, 400},
            .spot_light = {65, 5, 95, 35},
            .direction = motors_direction_t::DOWN_LEFT,
        };
    });
    MINI_MOCK_ON_CALL(motors_start_move_one_step, [](motors_direction_t motors_direction) {
        EXPECT(motors_direction == motors_direction_t::DOWN_LEFT);
    });
    state = sun_tracker_state_machine_update(sun_tracker_state_t::IDLE, sun_tracker_transition_t::START, drop, result);
    EXPECT(result == sun_tracker_result_t::UNKNOWN);
    EXPECT(state == sun_tracker_state_t::TRACKING);

    // From 'TRACKING' state with 'MOTORS_STOPPED' transition, when logic return a motors move
    MINI_MOCK_ON_CALL(sun_tracker_logic_detect, [](CImg<unsigned char> &image) {
        return sun_tracker_detection_t{
            .result = sun_tracker_detection_result_t::SUCCESS,
            .direction = motors_direction_t::DOWN,
        };
    });
    MINI_MOCK_ON_CALL(motors_start_move_one_step, [](motors_direction_t motors_direction) {
        EXPECT(motors_direction == motors_direction_t::DOWN);
    });
    state = sun_tracker_state_machine_update(
        sun_tracker_state_t::TRACKING, sun_tracker_transition_t::MOTORS_STOPPED, drop, result);
    EXPECT(result == sun_tracker_result_t::UNKNOWN);
    EXPECT(state == sun_tracker_state_t::TRACKING);

    // From 'TRACKING' state with 'MOTORS_STOPPED' transition, when logic return no motors move
    MINI_MOCK_ON_CALL(sun_tracker_logic_detect, [](CImg<unsigned char> &image) {
        return sun_tracker_detection_t{
            .result = sun_tracker_detection_result_t::SUCCESS,
        };
    });
    state = sun_tracker_state_machine_update(
        sun_tracker_state_t::TRACKING, sun_tracker_transition_t::MOTORS_STOPPED, drop, result);
    EXPECT(result == sun_tracker_result_t::SUCCESS);
    EXPECT(state == sun_tracker_state_t::IDLE);

    // From 'TRACKING' state with 'MOTORS_STOPPED' transition, when detection returns an error
    MINI_MOCK_ON_CALL(sun_tracker_logic_detect, [](CImg<unsigned char> &image) {
        return sun_tracker_detection_t{
            .result = sun_tracker_detection_result_t::SPOT_TOO_SMALL,
        };
    });
    state = sun_tracker_state_machine_update(
        sun_tracker_state_t::TRACKING, sun_tracker_transition_t::MOTORS_STOPPED, drop, result);
    EXPECT(result == sun_tracker_result_t::ERROR);
    EXPECT(state == sun_tracker_state_t::IDLE);

    // From 'TRACKING' state with 'STOP' transition
    state =
        sun_tracker_state_machine_update(sun_tracker_state_t::TRACKING, sun_tracker_transition_t::STOP, drop, result);
    EXPECT(result == sun_tracker_result_t::UNKNOWN);
    EXPECT(state == sun_tracker_state_t::STOPPING);

    // From 'TRACKING' state with 'STOP' and 'MOTORS_STOPPED' transitions
    sun_tracker_transition_t t = static_cast<sun_tracker_transition_t>(sun_tracker_transition_t::MOTORS_STOPPED
                                                                       | sun_tracker_transition_t::STOP);
    state = sun_tracker_state_machine_update(sun_tracker_state_t::TRACKING, t, drop, result);
    EXPECT(result == sun_tracker_result_t::ABORTED);
    EXPECT(state == sun_tracker_state_t::IDLE);
});

CREATE_MAIN_ENTRY_POINT();
