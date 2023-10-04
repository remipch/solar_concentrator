#include "image_processing.hpp"

#define LOG_LOCAL_LEVEL ESP_LOG_VERBOSE // ESP_LOG_INFO ESP_LOG_DEBUG ESP_LOG_VERBOSE
#include "esp_log.h"

#include "quirc.h"

// Tell CImg not to use display capabilities.
#undef cimg_display
#define cimg_display 0

#include "CImg.h"

using namespace cimg_library;

static const char *TAG = "image_processing";

static QueueHandle_t xQueueFrameI = NULL;
static QueueHandle_t xQueueEvent = NULL;
static QueueHandle_t xQueueFrameO = NULL;
static QueueHandle_t xQueueResult = NULL;

static struct quirc *capstone_detector;

static bool gEvent = true;

void rgb565ToRgb888(camera_fb_t *input, CImg<unsigned char>& output) {
    int i=0;
    for(int y=0; y<input->height; y++) {
        for(int x=0; x<input->width; x++) {
            // format is deduced from esp-who/components/esp-dl/include/image/dl_image.hpp
            uint16_t rgb565 = ((uint16_t*)input->buf)[i++];
            uint8_t b = (uint8_t)((rgb565 & 0x1F00) >> 5);
            uint8_t g = (uint8_t)(((rgb565 & 0x7) << 5) | ((rgb565 & 0xE000) >> 11));
            uint8_t r = (uint8_t)(rgb565 & 0xF8);
            *output.data(x,y,0,0) = r;
            *output.data(x,y,0,1) = g;
            *output.data(x,y,0,2) = b;
            if(i<40) {
                ESP_LOGV(TAG, "  x=%u y=%u rgb565=0x%04x -> r=0x%02x g=0x%02x b=0x%02x",x,y,rgb565,r,g,b);
            }
        }
    }
}

// cimg_library::CImg<unsigned char> grayscale_img = (0.2126 * img.channel(0)) + (0.7152 * img.channel(1)) + (0.0722 * img.channel(2))

// void rgb565ToGray8(camera_fb_t *input, CImg<unsigned char>& output) {
//     int i=0;
//     for(int y=0; y<input->height; y++) {
//         for(int x=0; x<input->width; x++) {
//             uint16_t rgb565 = ((uint16_t*)input->buf)[i++];
//             float b = (float)((rgb565 & 0x1F00) >> 5);
//             float g = (float)(((rgb565 & 0x7) << 5) | ((rgb565 & 0xE000) >> 11));
//             float r = (float)(rgb565 & 0xF8);
//             uint8_t gray = (uint8_t)((0.2126 * r) + (0.7152 * g) + (0.0722 * b));
//             *output.data(x,y,0) = gray;
//             if(i<40) {
//                 ESP_LOGV(TAG, "  x=%u y=%u rgb565=0x%04x -> gray=0x%02x",x,y,rgb565,gray);
//             }
//         }
//     }
// }
//
// void gray8ToRgb565(CImg<unsigned char>& input, camera_fb_t *output) {
//     int i=0;
//     for(int y=0; y<output->height; y++) {
//         for(int x=0; x<output->width; x++) {
//             uint8_t gray = *input.data(x,y,0);
//             uint16_t rgb565 = (gray&0xf8) | ((gray&0xe0)>>5)| ((gray&0x1c)<<11) | ((gray&0xf8)<<5);
//             ((uint16_t*)output->buf)[i++] = rgb565;
//             if(i<40) {
//                 ESP_LOGV(TAG, "  x=%u y=%u gray=0x%02x -> rgb565=0x%04x",x,y,gray,rgb565);
//             }
//         }
//     }
// }

void rgb888ToRgb565(CImg<unsigned char>& input, camera_fb_t *output) {
    int i=0;
    for(int y=0; y<input.height(); y++) {
        for(int x=0; x<input.width(); x++) {
            uint8_t r = *input.data(x,y,0,0);
            uint16_t g = (uint16_t)(*input.data(x,y,0,1));
            uint16_t b = (uint16_t)(*input.data(x,y,0,2));
            uint16_t rgb565 = (r&0xf8) | ((g&0xe0)>>5)| ((g&0x1c)<<11) | ((b&0xf8)<<5);
            ((uint16_t*)output->buf)[i++] = rgb565;
            if(i<40) {
                ESP_LOGV(TAG, "  x=%u y=%u r=0x%02x g=0x%02x b=0x%02x -> rgb565=0x%04x",x,y,r,g,b,rgb565);
            }
        }
    }
}

void rgb888ToQuircGrayscale(CImg<unsigned char>& input, uint8_t * output) {
    int i=0;
    for(int y=0; y<input.height(); y++) {
        for(int x=0; x<input.width(); x++) {
            float r = (float)(*input.data(x,y,0,0));
            float g = (float)(*input.data(x,y,0,1));
            float b = (float)(*input.data(x,y,0,2));
            uint8_t gray = (uint8_t)((0.2126 * r) + (0.7152 * g) + (0.0722 * b));
            output[i++] = gray;

            // Temp : change input to grayscale
            *input.data(x,y,0,0) = gray;
            *input.data(x,y,0,1) = gray;
            *input.data(x,y,0,2) = gray;

            if(i<40) {
                ESP_LOGV(TAG, "  x=%u y=%u gray=0x%02x",x,y,gray);
            }
        }
    }
}

void draw_something(camera_fb_t *frame) {
    ESP_LOGD(TAG, "Frame received (%u * %u)",(int)frame->width,(int)frame->height);

    // Create working image (allocate new buffer)
    CImg<unsigned char> img(frame->width,frame->height,1,3);
    rgb565ToRgb888(frame, img);

    // Draw something using CImg lib
    ESP_LOGD(TAG, "Draw");
    const unsigned char blue[] = { 0,0,128 };
    img.draw_line(5,0,5,70,blue).draw_ellipse(120,170,20,30,0,blue);
    const unsigned char green[] = { 0,255,0 };
    img.draw_line(4,0,4,70,green).draw_ellipse(110,160,20,30,0,green);
    const unsigned char red[] = { 255,0,0 };
    img.draw_line(3,0,3,70,red).draw_ellipse(100,150,20,30,0,red);
    const unsigned char white[] = { 255,255,255 };
    img.draw_text(10,10,"Hello",white,0,1,16);

    ESP_LOGD(TAG, "Convert back to rgb565");
    rgb888ToRgb565(img, frame);
}

void detect_target(camera_fb_t *frame) {
    ESP_LOGD(TAG, "Frame received (%u * %u)",(int)frame->width,(int)frame->height);

    // Create working image (allocate new buffer)
    CImg<unsigned char> img(frame->width,frame->height,1,3);

    ESP_LOGD(TAG, "Convert from frame_rgb565(c,x,y) to CImg_gray8(x,y,z,c)");
    rgb565ToRgb888(frame, img);

    uint8_t * quirc_image = quirc_begin(capstone_detector, NULL, NULL);
    rgb888ToQuircGrayscale(img, quirc_image);
    quirc_end(capstone_detector);

    int capstone_count = quirc_capstone_count(capstone_detector);
    ESP_LOGI(TAG, "%i capstone(s) detected", capstone_count);
    if(capstone_count>0) {
        const unsigned char red[] = { 255,0,0 };

        for(int i=0;i<capstone_count;i++) {
            printf("  capstone[%i]:\n",i);
            const struct quirc_capstone *capstone = quirc_get_capstone(capstone_detector,i);
            ESP_LOGV(TAG, "    capstone.center:  %i, %i", capstone->center.x, capstone->center.y);
            for(int i=0;i<4;i++) {
                ESP_LOGV(TAG, "    capstone.corners[%i]: %i , %i\n", i, capstone->corners[i].x, capstone->corners[i].y);
            }
            img.draw_line(capstone->center.x-20, capstone->center.y, capstone->center.x+20, capstone->center.y, red);
            img.draw_line(capstone->center.x, capstone->center.y-20, capstone->center.x, capstone->center.y+20, red);
        }
    }

    ESP_LOGD(TAG, "Convert back to rgb565");
    rgb888ToRgb565(img, frame);
}

static void task_process_handler(void *arg)
{
    // Note : frame is allocated by camera task and will be freed at the end of http tasks
    camera_fb_t *frame = NULL;

    while (true)
    {
        if (gEvent)
        {
            bool is_moved = false;
            if (xQueueReceive(xQueueFrameI, &(frame), portMAX_DELAY))
            {
                ESP_LOGD(TAG, "Frame received (%u * %u)",(int)frame->width,(int)frame->height);
                draw_something(frame);
            }

            if (xQueueFrameO)
            {
                xQueueSend(xQueueFrameO, &frame, portMAX_DELAY);
            }
            // Do not return frame because the buffer pointer is passed in the output queue
            // and following task must still have access to it
            // NO : esp_camera_fb_return(frame);

            if (xQueueResult)
            {
                xQueueSend(xQueueResult, &is_moved, portMAX_DELAY);
            }
        }
    }
}

static void task_event_handler(void *arg)
{
    while (true)
    {
        xQueueReceive(xQueueEvent, &(gEvent), portMAX_DELAY);
    }
}

void register_image_processing(QueueHandle_t frame_i, QueueHandle_t event,
                               QueueHandle_t result, QueueHandle_t frame_o)
{
    capstone_detector = quirc_new();

    if (quirc_resize(capstone_detector, 800, 600) < 0)
		ESP_LOGE(TAG, "quirc_resize fails");

    xQueueFrameI = frame_i;
    xQueueFrameO = frame_o;
    xQueueEvent = event;
    xQueueResult = result;
    xTaskCreatePinnedToCore(task_process_handler, TAG, 4 * 1024, NULL, 5, NULL, 1);
    if (xQueueEvent)
        xTaskCreatePinnedToCore(task_event_handler, TAG, 4 * 1024, NULL, 5, NULL, 1);
}
