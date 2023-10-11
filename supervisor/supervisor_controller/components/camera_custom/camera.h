#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

#include "esp_camera.h"

// Define constant image size and format so camera user components can pre-allocate memory
// before camera initialization
#define CAMERA_PIXEL_FORMAT PIXFORMAT_RGB565 // PIXFORMAT_RGB565 is required to serve capture_borders_uri (see app_httpd.cpp)
#define CAMERA_FRAMESIZE FRAMESIZE_SVGA
#define CAMERA_WIDTH 800
#define CAMERA_HEIGHT 600


#ifdef __cplusplus
extern "C"
{
#endif

    void register_camera(const QueueHandle_t frame_o);

#ifdef __cplusplus
}
#endif
