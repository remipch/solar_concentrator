#pragma once

#include "image.hpp"

void target_detector_init();

// return true if target has been successfully detected
bool target_detector_detect(CImg<unsigned char>& image, rectangle_t& target);
