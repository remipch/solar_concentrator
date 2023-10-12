#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "esp_camera.h"

void draw_something(camera_fb_t *frame);

void detect_target(camera_fb_t *frame);

void register_image_processing(QueueHandle_t frame_i, QueueHandle_t event,
                               QueueHandle_t result, QueueHandle_t frame_o = NULL);
