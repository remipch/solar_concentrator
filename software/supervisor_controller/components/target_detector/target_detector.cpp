// Copyright (C) 2024 Rémi Peuchot (https://remipch.github.io/)
// This code is distributed under GNU GPL v3 license

#include "esp_log.h"

#include "image.hpp"
#include "quirc.h"
#include "target_detector.hpp"

#include <assert.h>
#include <climits>

static const char *TAG = "target_detector";

static const int EXPECTED_CAPSTONE_COUNT = 4;
static const int MAX_CAPSTONE_COUNT = 10;
static const unsigned char BLACK = 0;
static const unsigned char WHITE = 255;

static const unsigned char MIN_CAPSTONE_SIZE = 25; // 10 cm capstone viewed at 3 meters
static const unsigned char MAX_CAPSTONE_SIZE = 60; // 10 cm capstone viewed at 1.5 meters

static const unsigned char MIN_PIXEL_THRESHOLD = 100;
static const unsigned char MAX_PIXEL_THRESHOLD = 180;
static const unsigned char PIXEL_THRESHOLD_STEP = 40;

static struct quirc *capstone_detector;

void target_detector_init() { capstone_detector = quirc_new(); }

template <typename T> struct quad {
    T top_left;
    T top_right;
    T bottom_left;
    T bottom_right;
};

// Convenient type for direct capstone geometric manipulation
struct capstone_geometry {
    int width;
    int height;
    quirc_point center;
    quad<quirc_point> corners;
};

void log_capstone(const capstone_geometry &geometry)
{
    ESP_LOGD(TAG, "capstone.center:  %i, %i", geometry.center.x, geometry.center.y);
    ESP_LOGD(TAG, "  capstone.size:  %i, %i", geometry.width, geometry.height);
    ESP_LOGD(TAG, "  capstone.top_left:  %i, %i", geometry.corners.top_left.x, geometry.corners.top_left.y);
    ESP_LOGD(TAG, "  capstone.top_right:  %i, %i", geometry.corners.top_right.x, geometry.corners.top_right.y);
    ESP_LOGD(TAG, "  capstone.bottom_left:  %i, %i", geometry.corners.bottom_left.x, geometry.corners.bottom_left.y);
    ESP_LOGD(TAG, "  capstone.bottom_right:  %i, %i", geometry.corners.bottom_right.x, geometry.corners.bottom_right.y);
}

void draw_capstone(CImg<unsigned char> &image, const capstone_geometry &capstone)
{
    image.draw_line(capstone.center.x - capstone.width / 2,
                    capstone.center.y,
                    capstone.center.x + capstone.width / 2,
                    capstone.center.y,
                    &WHITE);
    image.draw_line(capstone.center.x,
                    capstone.center.y - capstone.height / 2,
                    capstone.center.x,
                    capstone.center.y + capstone.height / 2,
                    &WHITE);
}

void log_target(const rectangle_t &target)
{
    ESP_LOGV(TAG, "target:  %i, %i, %i, %i", target.left_px, target.top_px, target.right_px, target.bottom_px);
}

void draw_target(CImg<unsigned char> &image, const rectangle_t &target)
{
    // Draw one pixel bigger than detected rectangle on each side because it will be cropped to extract target image
    // Note : here we know that the target rectangle is not at the exact image boudaries
    image.draw_rectangle(
        target.left_px - 1, target.top_px - 1, target.right_px + 1, target.bottom_px + 1, &WHITE, 1, 0xF0F0F0F0);
    image.draw_rectangle(
        target.left_px - 1, target.top_px - 1, target.right_px + 1, target.bottom_px + 1, &BLACK, 1, 0x0F0F0F0F);
}

// Extract ordered corners from capstone
quad<quirc_point> extract_corners(const quirc_capstone *capstone)
{
    int min_x = INT_MAX;
    int min_y = INT_MAX;
    int max_x = 0;
    int max_y = 0;
    for (int i = 0; i < 4; i++) {
        quirc_point corner = capstone->corners[i];
        min_x = std::min(min_x, corner.x);
        min_y = std::min(min_y, corner.y);
        max_x = std::max(max_x, corner.x);
        max_y = std::max(max_y, corner.y);
    }
    return quad<quirc_point>{
        .top_left = {min_x, min_y},
        .top_right = {max_x, min_y},
        .bottom_left = {min_x, max_y},
        .bottom_right = {max_x, max_y},
    };
}

capstone_geometry extract_capstone_geometry(const quirc_capstone *capstone)
{
    int min_x = INT_MAX;
    int min_y = INT_MAX;
    int max_x = 0;
    int max_y = 0;
    for (int i = 0; i < 4; i++) {
        quirc_point corner = capstone->corners[i];
        min_x = std::min(min_x, corner.x);
        min_y = std::min(min_y, corner.y);
        max_x = std::max(max_x, corner.x);
        max_y = std::max(max_y, corner.y);
    }
    quad<quirc_point> corners = {
        .top_left = {min_x, min_y},
        .top_right = {max_x, min_y},
        .bottom_left = {min_x, max_y},
        .bottom_right = {max_x, max_y},
    };
    return capstone_geometry{
        .width = (corners.top_right.x - corners.top_left.x + corners.bottom_right.x - corners.bottom_left.x) / 2,
        .height = (corners.bottom_left.y - corners.top_left.y + corners.bottom_right.y - corners.top_right.y) / 2,
        .center = capstone->center,
        .corners = corners,
    };
}

// Each capstones is supposed to be on one of the four corners,
// split them according to their place from average point
quad<const capstone_geometry *>
extract_capstones_quad(capstone_geometry capstones[EXPECTED_CAPSTONE_COUNT], int average_x, int average_y)
{

    quad<const capstone_geometry *> result{
        .top_left = NULL,
        .top_right = NULL,
        .bottom_left = NULL,
        .bottom_right = NULL,
    };

    for (int i = 0; i < EXPECTED_CAPSTONE_COUNT; i++) {
        capstone_geometry *capstone = &capstones[i];
        if (capstone->center.y < average_y) {
            if (capstone->center.x < average_x) {
                result.top_left = capstone;
            } else {
                result.top_right = capstone;
            }
        } else {
            if (capstone->center.x < average_x) {
                result.bottom_left = capstone;
            } else {
                result.bottom_right = capstone;
            }
        }
    }
    return result;
}

bool near_capstones(const capstone_geometry &geo1, const capstone_geometry &geo2)
{
    return std::abs(geo1.center.x - geo2.center.x) < std::min(geo1.width, geo2.width)
        && std::abs(geo1.center.y - geo2.center.y) < std::min(geo1.height, geo2.height);
}

bool target_detector_detect(CImg<unsigned char> &image, rectangle_t &target)
{
    // assert grayscale image
    assert(image.depth() == 1);
    assert(image.spectrum() == 1);

    int average_x = 0;
    int average_y = 0;
    int average_width = 0;
    int average_height = 0;

    // Store first capstone geometry for later use
    capstone_geometry capstones_geom[MAX_CAPSTONE_COUNT];

    // Parse detected capstones to :
    // - convert quirc_capstone to geomeetry
    // - compute usefull averages
    // - draw all detected capstones (for display purpose only)
    //   (capstones are drawn before checks to see what happen)
    int detected_capstone_count = 0;
    for (int threshold = MIN_PIXEL_THRESHOLD; threshold <= MAX_PIXEL_THRESHOLD; threshold += PIXEL_THRESHOLD_STEP) {
        int capstone_count =
            quirc_detect_capstones(capstone_detector, image.data(), image.width(), image.height(), threshold);

        for (int i = 0; i < capstone_count; i++) {
            const quirc_capstone *capstone = quirc_get_capstone(capstone_detector, i);
            capstone_geometry geometry = extract_capstone_geometry(capstone);

            // Ignore capstone if out of size
            if (geometry.width < MIN_CAPSTONE_SIZE || geometry.width > MAX_CAPSTONE_SIZE
                || geometry.height < MIN_CAPSTONE_SIZE || geometry.height > MAX_CAPSTONE_SIZE) {
                continue;
            }

            draw_capstone(image, geometry);

            // Ignore capstone if it has already been detected with a different threshold
            for (int j = 0; j < detected_capstone_count; j++) {
                if (near_capstones(capstones_geom[j], geometry)) {
                    continue;
                }
            }

            average_x += geometry.center.x;
            average_y += geometry.center.y;
            average_width += geometry.width;
            average_height += geometry.height;

            if (detected_capstone_count < MAX_CAPSTONE_COUNT) {
                capstones_geom[detected_capstone_count] = geometry;
                detected_capstone_count++;
            }
        }
    }

    // Check capstone count
    if (detected_capstone_count != EXPECTED_CAPSTONE_COUNT) {
        ESP_LOGW(TAG,
                 "Detection failed : %i capstone(s) detected instead of %i ",
                 detected_capstone_count,
                 EXPECTED_CAPSTONE_COUNT);
        for (int i = 0; i < detected_capstone_count; i++) {
            log_capstone(capstones_geom[i]);
        }
        return false;
    }

    average_x /= EXPECTED_CAPSTONE_COUNT;
    average_y /= EXPECTED_CAPSTONE_COUNT;
    average_width /= EXPECTED_CAPSTONE_COUNT;
    average_height /= EXPECTED_CAPSTONE_COUNT;

    quad<const capstone_geometry *> capstones = extract_capstones_quad(capstones_geom, average_x, average_y);

    // Here we should have exactly one capstone per corner
    if (capstones.top_left == NULL || capstones.top_right == NULL || capstones.bottom_left == NULL
        || capstones.bottom_right == NULL) {

        ESP_LOGW(TAG, "Detection failed : incorrect capstones placement");
        return false;
    }

    // Check misalignment < average_size
    if (std::abs(capstones.top_left->center.x - capstones.bottom_left->center.x) > average_width
        || std::abs(capstones.top_right->center.x - capstones.bottom_right->center.x) > average_width
        || std::abs(capstones.top_left->center.y - capstones.top_right->center.y) > average_height
        || std::abs(capstones.bottom_left->center.y - capstones.bottom_right->center.y) > average_height) {

        ESP_LOGW(TAG, "Detection failed : incorrect capstones alignment");
        return false;
    }

    // Compute target area rectangle from capstones geometry (see schema in README)
    target = {
        .left_px = std::max(capstones.top_left->center.x, capstones.bottom_left->center.x) - average_width / 2,
        .top_px = std::max(capstones.top_left->center.y, capstones.top_right->center.y) + 2 * average_height,
        .right_px = std::min(capstones.top_right->center.x, capstones.bottom_right->center.x) + average_width / 2,
        .bottom_px = std::min(capstones.bottom_left->center.y, capstones.bottom_right->center.y) - 2 * average_height,
    };

    // Check area_size > average capstone size
    if ((target.bottom_px - target.top_px) < average_height || (target.right_px - target.left_px) < average_width) {

        ESP_LOGW(TAG, "Detection failed : target area too small");
        return false;
    }

    log_target(target);
    draw_target(image, target);

    return true;
}
