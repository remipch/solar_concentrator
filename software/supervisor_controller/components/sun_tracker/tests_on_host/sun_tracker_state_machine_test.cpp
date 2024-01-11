#include "mini_mock.hpp"

#include "image.hpp"
#include "motors.hpp"
#include "sun_tracker_logic.hpp"
#include "sun_tracker_state_machine.hpp"

MINI_MOCK_FUNCTION(camera_capture,
                   bool,
                   (bool drop_current_image, CImg<unsigned char> &grayscale_cimg),
                   (drop_current_image, grayscale_cimg));
MINI_MOCK_FUNCTION(target_detector_detect, bool, (CImg<unsigned char> & image, rectangle_t &target), (image, target));
MINI_MOCK_FUNCTION(sun_tracker_logic_start,
                   sun_tracker_logic_result_t,
                   (CImg<unsigned char> & target_img, motors_direction_t &motors_direction),
                   (target_img, motors_direction));
MINI_MOCK_FUNCTION(sun_tracker_logic_update,
                   sun_tracker_logic_result_t,
                   (CImg<unsigned char> & target_img, motors_direction_t &motors_direction),
                   (target_img, motors_direction));
MINI_MOCK_FUNCTION(motors_start_move_one_step, void, (motors_direction_t direction), (direction));

// sun_tracker image callbacks are for display purpose only,
void drop(CImg<unsigned char> &img) {}

TEST(typical_scenario, []() {
    CImg<unsigned char> img;

    // Define common dummy mocks used for all tests bellow
    MINI_MOCK_ON_CALL(
        camera_capture,
        [](bool drop_current_image, CImg<unsigned char> &img) {
            // return dummy black image (not used because sun_tracker_logic is mocked)
            img = CImg<unsigned char>(CAMERA_WIDTH, CAMERA_HEIGHT, 1, 1, 0);
            return true;
        },
        7);
    MINI_MOCK_ON_CALL(
        target_detector_detect,
        [](CImg<unsigned char> &image, rectangle_t &target) {
            target = {
                .left_px = 100,
                .top_px = 200,
                .right_px = 300,
                .bottom_px = 400,
            };
            return true;
        },
        7);

    // From 'UNINITIALIZED' state
    sun_tracker_state_t state = sun_tracker_state_machine_update(
        sun_tracker_state_t::UNINITIALIZED, sun_tracker_transition_t::NONE, drop, drop);
    EXPECT(state == sun_tracker_state_t::IDLE);

    // From 'IDLE' state without transition
    state = sun_tracker_state_machine_update(sun_tracker_state_t::IDLE, sun_tracker_transition_t::NONE, drop, drop);
    EXPECT(state == sun_tracker_state_t::IDLE);

    // From 'IDLE' state with 'START' transition, when logic return 'TARGET_REACHED'
    MINI_MOCK_ON_CALL(sun_tracker_logic_start,
                      [](CImg<unsigned char> &target_img, motors_direction_t &motors_direction) {
                          motors_direction = motors_direction_t::NONE;
                          return sun_tracker_logic_result_t::TARGET_REACHED;
                      });
    state = sun_tracker_state_machine_update(sun_tracker_state_t::IDLE, sun_tracker_transition_t::START, drop, drop);
    EXPECT(state == sun_tracker_state_t::SUCCESS);

    // From 'IDLE' state with 'START' transition, when logic return 'MUST_MOVE'
    MINI_MOCK_ON_CALL(sun_tracker_logic_start,
                      [](CImg<unsigned char> &target_img, motors_direction_t &motors_direction) {
                          motors_direction = motors_direction_t::DOWN_LEFT;
                          return sun_tracker_logic_result_t::MUST_MOVE;
                      });
    MINI_MOCK_ON_CALL(motors_start_move_one_step, [](motors_direction_t motors_direction) {
        EXPECT(motors_direction == motors_direction_t::DOWN_LEFT);
    });
    state = sun_tracker_state_machine_update(sun_tracker_state_t::IDLE, sun_tracker_transition_t::START, drop, drop);
    EXPECT(state == sun_tracker_state_t::TRACKING);

    // From 'IDLE' state with 'START' transition, when logic return an error
    MINI_MOCK_ON_CALL(sun_tracker_logic_start,
                      [](CImg<unsigned char> &target_img, motors_direction_t &motors_direction) {
                          return sun_tracker_logic_result_t::NO_SPOT_DETECTED;
                      });
    state = sun_tracker_state_machine_update(sun_tracker_state_t::IDLE, sun_tracker_transition_t::START, drop, drop);
    EXPECT(state == sun_tracker_state_t::ERROR);

    // From 'TRACKING' state with 'MOTORS_STOPPED' transition, when logic return 'MUST_MOVE'
    MINI_MOCK_ON_CALL(sun_tracker_logic_update,
                      [](CImg<unsigned char> &target_img, motors_direction_t &motors_direction) {
                          motors_direction = motors_direction_t::DOWN;
                          return sun_tracker_logic_result_t::MUST_MOVE;
                      });
    MINI_MOCK_ON_CALL(motors_start_move_one_step, [](motors_direction_t motors_direction) {
        EXPECT(motors_direction == motors_direction_t::DOWN);
    });
    state = sun_tracker_state_machine_update(
        sun_tracker_state_t::TRACKING, sun_tracker_transition_t::MOTORS_STOPPED, drop, drop);
    EXPECT(state == sun_tracker_state_t::TRACKING);

    // From 'TRACKING' state with 'MOTORS_STOPPED' transition, when logic return 'TARGET_REACHED'
    MINI_MOCK_ON_CALL(sun_tracker_logic_update,
                      [](CImg<unsigned char> &target_img, motors_direction_t &motors_direction) {
                          motors_direction = motors_direction_t::NONE;
                          return sun_tracker_logic_result_t::TARGET_REACHED;
                      });
    state = sun_tracker_state_machine_update(
        sun_tracker_state_t::TRACKING, sun_tracker_transition_t::MOTORS_STOPPED, drop, drop);
    EXPECT(state == sun_tracker_state_t::SUCCESS);

    // From 'TRACKING' state with 'MOTORS_STOPPED' transition, when logic return an error
    MINI_MOCK_ON_CALL(sun_tracker_logic_update,
                      [](CImg<unsigned char> &target_img, motors_direction_t &motors_direction) {
                          return sun_tracker_logic_result_t::SPOT_TOO_BIG;
                      });
    state = sun_tracker_state_machine_update(
        sun_tracker_state_t::TRACKING, sun_tracker_transition_t::MOTORS_STOPPED, drop, drop);
    EXPECT(state == sun_tracker_state_t::ERROR);

    // From 'TRACKING' state with 'STOP' transition
    state = sun_tracker_state_machine_update(sun_tracker_state_t::TRACKING, sun_tracker_transition_t::STOP, drop, drop);
    EXPECT(state == sun_tracker_state_t::STOPPING);

    // From 'TRACKING' state with 'STOP' and 'MOTORS_STOPPED' transitions
    sun_tracker_transition_t t = static_cast<sun_tracker_transition_t>(sun_tracker_transition_t::MOTORS_STOPPED
                                                                       | sun_tracker_transition_t::STOP);
    state = sun_tracker_state_machine_update(sun_tracker_state_t::TRACKING, t, drop, drop);
    EXPECT(state == sun_tracker_state_t::IDLE);

    // From 'ERROR' state with 'RESET' transition
    state = sun_tracker_state_machine_update(sun_tracker_state_t::ERROR, sun_tracker_transition_t::RESET, drop, drop);
    EXPECT(state == sun_tracker_state_t::IDLE);

    // From 'SUCCESS' state with 'RESET' transition
    state = sun_tracker_state_machine_update(sun_tracker_state_t::ERROR, sun_tracker_transition_t::RESET, drop, drop);
    EXPECT(state == sun_tracker_state_t::IDLE);
});

CREATE_MAIN_ENTRY_POINT();
