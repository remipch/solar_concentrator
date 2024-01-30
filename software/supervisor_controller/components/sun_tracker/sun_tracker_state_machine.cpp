#include "sun_tracker_state_machine.hpp"
#include "sun_tracker_logic.hpp"

#include "esp_log.h" // replaced by stub/esp_log.h for tests on host

#include "camera.hpp"
#include "motors.hpp"
#include "target_detector.hpp"

static const char *TAG = "sun_tracker_state_machine";

// Camera full image is created statically to avoid future memory allocations
static CImg<unsigned char> full_img(CAMERA_WIDTH, CAMERA_HEIGHT, 1, 1);

static rectangle_t target_area;

static motors_direction_t motors_direction;

sun_tracker_state_t sun_tracker_state_machine_update(sun_tracker_state_t current_state,
                                                     sun_tracker_transition_t transition,
                                                     sun_tracker_image_callback publish_full_image,
                                                     sun_tracker_image_callback publish_target_image,
                                                     sun_tracker_logic_result_t &logic_result)
{
    logic_result = sun_tracker_logic_result_t::UNKNOWN;

    if (current_state == sun_tracker_state_t::UNINITIALIZED) {
        // Nothing to do but signal state_machine has started by quitting UNINITIALIZED state
        motors_direction = motors_direction_t::NONE;
        return sun_tracker_state_t::IDLE;
    }

    if (current_state == sun_tracker_state_t::IDLE) {
        if (!camera_capture(false, full_img)) {
            ESP_LOGE(TAG, "Camera capture failed");
            return sun_tracker_state_t::ERROR;
        }

        bool target_detected = target_detector_detect(full_img, target_area);
        if (!target_detected) {
            ESP_LOGW(TAG, "target_detector failed");
        }

        // Publish full image even if target has not been detected for debug purpose
        publish_full_image(full_img);

        if (!target_detected) {
            // Stay in IDLE state to allow user to fix target
            return sun_tracker_state_t::IDLE;
        }

        // Extract and publish target area image
        CImg<unsigned char> target_img =
            full_img.get_crop(target_area.left_px, target_area.top_px, target_area.right_px, target_area.bottom_px);

        if (transition & sun_tracker_transition_t::START) {
            logic_result = sun_tracker_logic_start(target_img, motors_direction);

            // Publish target image here because logic can draw usefull things
            publish_target_image(target_img);

            if (logic_result == sun_tracker_logic_result_t::TARGET_REACHED) {
                return sun_tracker_state_t::SUCCESS;
            } else if (logic_result == sun_tracker_logic_result_t::MUST_MOVE) {
                motors_start_move_one_step(motors_direction);
                return sun_tracker_state_t::TRACKING;
            } else {
                return sun_tracker_state_t::ERROR;
            }
        }
        publish_target_image(target_img);
    }

    if (current_state == sun_tracker_state_t::TRACKING) {
        if (transition & sun_tracker_transition_t::STOP) {
            if (transition & sun_tracker_transition_t::MOTORS_STOPPED) {
                // Particular case when both transitions have been triggered
                return sun_tracker_state_t::IDLE;
            }
            return sun_tracker_state_t::STOPPING;
        }
        if (transition & sun_tracker_transition_t::MOTORS_STOPPED) {

            if (!camera_capture(false, full_img)) {
                ESP_LOGE(TAG, "Camera capture failed");
                return sun_tracker_state_t::ERROR;
            }

            rectangle_t new_target_area;
            bool target_detected = target_detector_detect(full_img, new_target_area);
            if (!target_detected) {
                ESP_LOGW(TAG, "target_detector failed");
            }

            // Publish full image even if target has not been detected for debug purpose
            publish_full_image(full_img);

            // TODO : check that target has not moved too much
            // Note : keep the initial target area for consistency between sun_tracker_logic calls
            if (!target_detected) {
                return sun_tracker_state_t::ERROR;
            }

            // Extract and publish target area image
            CImg<unsigned char> target_img =
                full_img.get_crop(target_area.left_px, target_area.top_px, target_area.right_px, target_area.bottom_px);

            logic_result = sun_tracker_logic_update(target_img, motors_direction);

            // Publish target image here because logic can draw usefull things
            publish_target_image(target_img);

            if (logic_result == sun_tracker_logic_result_t::TARGET_REACHED) {
                return sun_tracker_state_t::SUCCESS;
            } else if (logic_result == sun_tracker_logic_result_t::MUST_MOVE) {
                motors_start_move_one_step(motors_direction);
                return sun_tracker_state_t::TRACKING;
            } else {
                return sun_tracker_state_t::ERROR;
            }
        }
    }

    if (current_state == sun_tracker_state_t::STOPPING) {
        if (transition & sun_tracker_transition_t::MOTORS_STOPPED) {
            return sun_tracker_state_t::IDLE;
        }
    }

    if (current_state == sun_tracker_state_t::ERROR || current_state == sun_tracker_state_t::SUCCESS) {
        if (transition & sun_tracker_transition_t::RESET) {
            return sun_tracker_state_t::IDLE;
        }
    }

    if (transition != sun_tracker_transition_t::NONE) {
        ESP_LOGW(TAG, "Nothing to do for state: '%s', transition: '%s'", str(current_state), str(transition));
    }
    return current_state;
}
