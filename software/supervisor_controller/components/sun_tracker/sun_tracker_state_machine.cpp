#include "sun_tracker_state_machine.hpp"

#include "esp_log.h" // replaced by stub/esp_log.h for tests on host

#include "camera.hpp"
#include "motors.hpp"
#include "target_detector.hpp"

static const char *TAG = "sun_tracker_state_machine";

// Camera full image is created statically to avoid future memory allocations
static CImg<unsigned char> grayscale_cimg_full(CAMERA_WIDTH, CAMERA_HEIGHT, 1, 1);

sun_tracker_state_t sun_tracker_state_machine_update(sun_tracker_state_t current_state,
                                                     sun_tracker_transition_t transition,
                                                     sun_tracker_image_callback publish_full_image,
                                                     sun_tracker_image_callback publish_target_image)
{

    if (current_state == sun_tracker_state_t::UNINITIALIZED) {
        // Nothing to do but signal state_machine has started by quitting UNINITIALIZED state
        return sun_tracker_state_t::IDLE;
    }

    if (current_state == sun_tracker_state_t::IDLE) {
        if (!camera_capture(false, grayscale_cimg_full)) {
            ESP_LOGE(TAG, "Camera capture failed");
            return sun_tracker_state_t::ERROR;
        }

        rectangle_t target_area;
        if (!target_detector_detect(grayscale_cimg_full, target_area)) {
            ESP_LOGW(TAG, "target_detector failed");
        }

        publish_full_image(grayscale_cimg_full);

        // Stay in same state
        return current_state;
    }

    if (transition != sun_tracker_transition_t::NONE) {
        ESP_LOGW(TAG, "Nothing to do for state: '%s', transition: '%s'", str(current_state), str(transition));
    }
    return current_state;
}
