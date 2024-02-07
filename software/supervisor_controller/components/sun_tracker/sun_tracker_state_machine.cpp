#include "sun_tracker_state_machine.hpp"
#include "sun_tracker_logic.hpp"

#include "esp_log.h" // replaced by stub/esp_log.h for tests on host

#include "camera.hpp"
#include "motors.hpp"

static const char *TAG = "sun_tracker_state_machine";

// Camera full image is created statically to avoid future memory allocations
static CImg<unsigned char> full_img(CAMERA_WIDTH, CAMERA_HEIGHT, 1, 1);

static sun_tracker_detection_t detection_before_move;

static sun_tracker_detection_result_t last_detection_result = sun_tracker_detection_result_t::UNKNOWN;

sun_tracker_detection_result_t sun_tracker_state_machine_get_detection_result() { return last_detection_result; }

sun_tracker_state_t sun_tracker_state_machine_update(sun_tracker_state_t current_state,
                                                     sun_tracker_transition_t transition,
                                                     panel_t panel,
                                                     sun_tracker_image_callback publish_full_image,
                                                     sun_tracker_result_t &result)
{
    result = sun_tracker_result_t::UNKNOWN;

    if (current_state == sun_tracker_state_t::UNINITIALIZED) {
        // Nothing to do but signal state_machine has started by quitting UNINITIALIZED state
        return sun_tracker_state_t::IDLE;
    }

    if (current_state == sun_tracker_state_t::IDLE) {
        if (!camera_capture(false, full_img)) {
            ESP_LOGE(TAG, "Camera capture failed");
            result = sun_tracker_result_t::ERROR;
            return sun_tracker_state_t::IDLE;
        }

        detection_before_move = sun_tracker_logic_detect(full_img);
        last_detection_result = detection_before_move.result;

        // Publish full image after detection for debug purpose
        publish_full_image(full_img);

        if (detection_before_move.result != sun_tracker_detection_result_t::SUCCESS) {
            // Stay in IDLE state to allow user to fix target or spot
            result = sun_tracker_result_t::ERROR;
            return sun_tracker_state_t::IDLE;
        }

        if (transition & sun_tracker_transition_t::START) {
            if (detection_before_move.direction == motors_direction_t::NONE) {
                result = sun_tracker_result_t::SUCCESS;
                return sun_tracker_state_t::IDLE;
            } else {
                motors_start_move_one_step(panel, detection_before_move.direction);
                return sun_tracker_state_t::TRACKING;
            }
        }
    }

    if (current_state == sun_tracker_state_t::TRACKING) {
        if (transition & sun_tracker_transition_t::STOP) {
            if (transition & sun_tracker_transition_t::MOTORS_STOPPED) {
                // Particular case when both transitions have been triggered
                result = sun_tracker_result_t::ABORTED;
                return sun_tracker_state_t::IDLE;
            }
            return sun_tracker_state_t::STOPPING;
        }
        if (transition & sun_tracker_transition_t::MOTORS_STOPPED) {
            if (!camera_capture(true, full_img)) {
                ESP_LOGE(TAG, "Camera capture failed");
                result = sun_tracker_result_t::ERROR;
                return sun_tracker_state_t::IDLE;
            }

            sun_tracker_detection_t detection_after_move = sun_tracker_logic_detect(full_img);
            last_detection_result = detection_after_move.result;

            // Publish full image after detection for debug purpose
            publish_full_image(full_img);

            // TODO : check that target has not moved too much
            if (detection_after_move.result != sun_tracker_detection_result_t::SUCCESS) {
                result = sun_tracker_result_t::ERROR;
                return sun_tracker_state_t::IDLE;
            }

            motors_direction_t direction = sun_tracker_logic_update(detection_before_move, detection_after_move);

            if (direction == motors_direction_t::NONE) {
                result = sun_tracker_result_t::SUCCESS;
                return sun_tracker_state_t::IDLE;
            } else {
                motors_start_move_one_step(panel, direction);
                return sun_tracker_state_t::TRACKING;
            }
        }
    }

    if (current_state == sun_tracker_state_t::STOPPING) {
        if (transition & sun_tracker_transition_t::MOTORS_STOPPED) {
            result = sun_tracker_result_t::ABORTED;
            return sun_tracker_state_t::IDLE;
        }
    }

    if (transition != sun_tracker_transition_t::NONE) {
        ESP_LOGW(TAG, "Nothing to do for state: '%s', transition: '%s'", str(current_state), str(transition));
    }
    return current_state;
}
