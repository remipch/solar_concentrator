#include "image_processing.hpp"

#define LOG_LOCAL_LEVEL ESP_LOG_INFO // ESP_LOG_INFO ESP_LOG_DEBUG ESP_LOG_VERBOSE
#include "esp_log.h"
#include "esp_camera.h"

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

static bool gEvent = true;

void logFirstPixels(unsigned char* buf) {
    for(int i=0; i<60; i+=6) {
        ESP_LOGV(TAG, "  0x%02x %02x %02x %02x %02x %02x",buf[i],buf[i+1],buf[i+2],buf[i+3],buf[i+4],buf[i+5]);
    }
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
                logFirstPixels(frame->buf);

                // Create working image (allocate new buffer)
                CImg<unsigned char> img(frame->width,frame->height,1,3);
                logFirstPixels(img.data());

                ESP_LOGD(TAG, "Convert from frame_rgb565(c,x,y) to CImg_rgb888(x,y,z,c)");
                int i=0;
                for(int y=0; y<frame->height; y++) {
                    for(int x=0; x<frame->width; x++) {
                        // format is deduced from /home/remi/esp/esp-who/components/esp-dl/include/image/dl_image.hpp
                        uint16_t rgb565 = ((uint16_t*)frame->buf)[i++];
                        uint8_t b = (uint8_t)((rgb565 & 0x1F00) >> 5);
                        uint8_t g = (uint8_t)(((rgb565 & 0x7) << 5) | ((rgb565 & 0xE000) >> 11));
                        uint8_t r = (uint8_t)(rgb565 & 0xF8);
                        *img.data(x,y,0,0) = r;
                        *img.data(x,y,0,1) = g;
                        *img.data(x,y,0,2) = b;
                        if(i<40) {
                            ESP_LOGV(TAG, "  x=%u y=%u rgb565=0x%04x",x,y,rgb565);
                            ESP_LOGV(TAG, "  r=0x%02x g=0x%02x b=0x%02x",r,g,b);
                            ESP_LOGV(TAG, "  data[x,y,0,0]=%02x data[i]=%02x offset=%ld",
                                     *img.data(x,y,0,0),img.data()[i],img.offset(x,y,0,0));
                        }
                    }
                }
                logFirstPixels(img.data());

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
                logFirstPixels(img.data());

                ESP_LOGD(TAG, "Convert back to rgb565");
                i=0;
                for(int y=0; y<frame->height; y++) {
                    for(int x=0; x<frame->width; x++) {
                        uint8_t r = *img.data(x,y,0,0);
                        uint16_t g = (uint16_t)(*img.data(x,y,0,1));
                        uint16_t b = (uint16_t)(*img.data(x,y,0,2));
                        uint16_t rgb565 = (r&0xf8) | ((g&0xe0)>>5)| ((g&0x1c)<<11) | ((b&0xf8)<<5);
                        ((uint16_t*)frame->buf)[i++] = rgb565;
                        if(i<40) {
                            ESP_LOGV(TAG, "  x=%u y=%u rgb565=0x%04x",x,y,rgb565);
                            ESP_LOGV(TAG, "  r=0x%02x g=0x%02x b=0x%02x",r,g,b);
                        }
                    }
                }
                logFirstPixels(frame->buf);
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
    xQueueFrameI = frame_i;
    xQueueFrameO = frame_o;
    xQueueEvent = event;
    xQueueResult = result;
    xTaskCreatePinnedToCore(task_process_handler, TAG, 4 * 1024, NULL, 5, NULL, 1);
    if (xQueueEvent)
        xTaskCreatePinnedToCore(task_event_handler, TAG, 4 * 1024, NULL, 5, NULL, 1);
}
