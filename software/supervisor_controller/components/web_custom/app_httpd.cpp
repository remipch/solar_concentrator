// Copyright 2015-2016 Espressif Systems (Shanghai) PTE LTD
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "app_httpd.hpp"

#include "esp_http_server.h"
#include "img_converters.h"
#include "sdkconfig.h"
#include <list>

#include "camera.hpp"
#include "motors.hpp" // to display motor state
#include "sun_tracker.hpp"
#include "supervisor.hpp"

#if defined(ARDUINO_ARCH_ESP32) && defined(CONFIG_ARDUHAL_ESP_LOG)
#include "esp32-hal-log.h"
#define TAG ""
#else
#include "esp_log.h"
static const char *TAG = "app_httpd";
#endif

static QueueHandle_t xQueueFrameI = NULL;
static QueueHandle_t xQueueFrameO = NULL;
static bool gReturnFB = true;

typedef struct {
    httpd_req_t *req;
    size_t len;
} jpg_chunking_t;

#define PART_BOUNDARY "123456789000000000000987654321"
static const char *_STREAM_CONTENT_TYPE = "multipart/x-mixed-replace;boundary=" PART_BOUNDARY;
static const char *_STREAM_BOUNDARY = "\r\n--" PART_BOUNDARY "\r\n";
static const char *_STREAM_PART = "Content-Type: image/jpeg\r\nContent-Length: %u\r\nX-Timestamp: %d.%06d\r\n\r\n";

httpd_handle_t stream_httpd = NULL;
httpd_handle_t camera_httpd = NULL;

static size_t jpg_encode_stream(void *arg, size_t index, const void *data, size_t len)
{
    jpg_chunking_t *j = (jpg_chunking_t *)arg;
    if (!index) {
        j->len = 0;
    }
    if (httpd_resp_send_chunk(j->req, (const char *)data, len) != ESP_OK) {
        return 0;
    }
    j->len += len;
    return len;
}

static esp_err_t parse_get(httpd_req_t *req, char **obuf)
{
    char *buf = NULL;
    size_t buf_len = 0;

    buf_len = httpd_req_get_url_query_len(req) + 1;
    if (buf_len > 1) {
        buf = (char *)malloc(buf_len);
        if (!buf) {
            httpd_resp_send_500(req);
            return ESP_FAIL;
        }
        if (httpd_req_get_url_query_str(req, buf, buf_len) == ESP_OK) {
            *obuf = buf;
            return ESP_OK;
        }
        free(buf);
    }
    httpd_resp_send_404(req);
    return ESP_FAIL;
}

static uint8_t *buf = (uint8_t *)malloc(CAMERA_WIDTH * CAMERA_HEIGHT);

static CImg<unsigned char> img(CAMERA_WIDTH, CAMERA_HEIGHT, 1, 1);

static esp_err_t capture_handler(httpd_req_t *req)
{
    esp_err_t res = ESP_OK;

    ESP_LOGI(TAG, "capture_handler");

    // Temp test

    ESP_LOGI(TAG, "frame created");

    if (!camera_capture(false, img)) {
        ESP_LOGE(TAG, "Camera capture failed");
        httpd_resp_send_500(req);
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "img captured");

    camera_fb_t frame = grayscale_cimg_to_grayscale_frame(img, buf);

    ESP_LOGI(TAG, "img converted");

    //     detect_target(frame);

    httpd_resp_set_type(req, "image/jpeg");
    httpd_resp_set_hdr(req, "Content-Disposition", "inline; filename=capture.jpg");
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");

    //         char ts[32];
    //         snprintf(ts, 32, "%lld.%06ld", frame.timestamp.tv_sec, frame.timestamp.tv_usec);
    //         httpd_resp_set_hdr(req, "X-Timestamp", (const char *)ts);

    // size_t fb_len = 0;
    if (frame.format == PIXFORMAT_JPEG) {
        // fb_len = frame.len;
        res = httpd_resp_send(req, (const char *)frame.buf, frame.len);
    } else {
        jpg_chunking_t jchunk = {req, 0};
        res = frame2jpg_cb(&frame, 80, jpg_encode_stream, &jchunk) ? ESP_OK : ESP_FAIL;
        httpd_resp_send_chunk(req, NULL, 0);
        // fb_len = jchunk.len;
    }

    return res;
}

// TODO: move image conversion stuff in custom camera component
static esp_err_t capture_area_handler(httpd_req_t *req)
{
    camera_fb_t *frame = NULL;
    esp_err_t res = ESP_OK;

    ESP_LOGI(TAG, "capture_area_handler");

    // Ugly workarround to force the camera to take a new image now
    // otherwise it returns the frame in the queue, which is one frame late
    ESP_LOGV(TAG, "esp_camera_fb_get 0");
    frame = esp_camera_fb_get();
    esp_camera_fb_return(frame);
    ESP_LOGV(TAG, "esp_camera_fb_get 1");
    frame = esp_camera_fb_get();
    ESP_LOGV(TAG, "esp_camera_fb_get 2");

    char *buf = NULL;
    char left_px_str[32];
    char top_px_str[32];
    char right_px_str[32];
    char bottom_px_str[32];

    if (parse_get(req, &buf) != ESP_OK
        || httpd_query_key_value(buf, "left_px", left_px_str, sizeof(left_px_str)) != ESP_OK
        || httpd_query_key_value(buf, "top_px", top_px_str, sizeof(top_px_str)) != ESP_OK
        || httpd_query_key_value(buf, "right_px", right_px_str, sizeof(right_px_str)) != ESP_OK
        || httpd_query_key_value(buf, "bottom_px", bottom_px_str, sizeof(bottom_px_str)) != ESP_OK) {
        free(buf);
        httpd_resp_send_404(req);
        return ESP_FAIL;
    }
    free(buf);

    int left_px = atoi(left_px_str);
    int top_px = atoi(top_px_str);
    int right_px = atoi(right_px_str);
    int bottom_px = atoi(bottom_px_str);

    // Check consistency
    if (left_px >= right_px || left_px < 0 || left_px >= frame->width || right_px < 0 || right_px >= frame->width
        || top_px >= bottom_px || top_px < 0 || top_px >= frame->height || bottom_px < 0
        || bottom_px >= frame->height) {
        ESP_LOGE(TAG, "Inconsistent area");
        httpd_resp_send_404(req);
        return ESP_FAIL;
    }

    int area_width = right_px - left_px + 1;
    int area_height = bottom_px - top_px + 1;
    uint8_t *area_gray = (uint8_t *)malloc(area_width * area_height);

    ESP_LOGD(TAG, "Convert from RGB565 to gray");
    for (int y = top_px; y <= bottom_px; y++) {
        for (int x = left_px; x <= right_px; x++) {
            // format is deduced from esp-who/components/esp-dl/include/image/dl_image.hpp
            int source_index = x + y * frame->width;
            uint16_t rgb565 = ((uint16_t *)frame->buf)[source_index];
            uint8_t b = (uint8_t)((rgb565 & 0x1F00) >> 5);
            uint8_t g = (uint8_t)(((rgb565 & 0x7) << 5) | ((rgb565 & 0xE000) >> 11));
            uint8_t r = (uint8_t)(rgb565 & 0xF8);

            int area_index = x - left_px + (y - top_px) * area_width;
            uint8_t gray = (0.299 * r) + (0.587 * g) + (0.114 * b);
            area_gray[area_index] = gray;
        }
    }

    char ts[32];
    snprintf(ts, 32, "%lld.%06ld", frame->timestamp.tv_sec, frame->timestamp.tv_usec);
    httpd_resp_set_hdr(req, "X-Timestamp", (const char *)ts);

    // size_t fb_len = 0;
    if (frame->format == PIXFORMAT_RGB565) {
        httpd_resp_set_type(req, "application/octet-stream");
        httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
        res = httpd_resp_send(req, (const char *)area_gray, area_width * area_height);
    } else {
        ESP_LOGE(TAG, "Unexpected pixel format : cannot serve capture_area_handler");
    }

    free(area_gray);

    esp_camera_fb_return(frame);

    return res;
}

static uint8_t *full_image_buffer = (uint8_t *)malloc(CAMERA_WIDTH * CAMERA_HEIGHT);
static camera_fb_t full_image_frame{};
static SemaphoreHandle_t full_image_mutex; // mutual exclusion of read/write to full_image_frame and its buffer
static SemaphoreHandle_t full_image_ready; // signal by full_image_updated callback when a new image has been updated
static const int full_image_mutex_TIMEOUT_MS = 5000;

// Note : the caller guarantee that full_image object is not changed until this function returns
void full_image_updated(CImg<unsigned char> &full_image)
{
    ESP_LOGI(TAG, "full_image_updated: %i x %i", full_image.width(), full_image.height());
    assert(xSemaphoreTake(full_image_mutex, pdMS_TO_TICKS(full_image_mutex_TIMEOUT_MS)));
    full_image_frame = grayscale_cimg_to_grayscale_frame(full_image, full_image_buffer);
    xSemaphoreGive(full_image_mutex);
    xSemaphoreGive(full_image_ready);
}

static esp_err_t stream_handler(httpd_req_t *req)
{
    esp_err_t res = httpd_resp_set_type(req, _STREAM_CONTENT_TYPE);
    if (res != ESP_OK) {
        return res;
    }

    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    httpd_resp_set_hdr(req, "X-Framerate", "60");

    while (true) {
        if (!xSemaphoreTake(full_image_ready, pdMS_TO_TICKS(full_image_mutex_TIMEOUT_MS))) {
            ESP_LOGE(TAG, "Full image not updated on time");
            return ESP_FAIL;
        }

        size_t _jpg_buf_len = 0;
        uint8_t *_jpg_buf = NULL;
        char *part_buf[128];

        assert(xSemaphoreTake(full_image_mutex, pdMS_TO_TICKS(full_image_mutex_TIMEOUT_MS)));
        // Use or cache full_image_frame data inside mutex-protected bloc, it can be sent outside of this bloc
        pixformat_t format = full_image_frame.format;

        if (format == PIXFORMAT_JPEG) {
            _jpg_buf = full_image_frame.buf;
            _jpg_buf_len = full_image_frame.len;
        } else if (!frame2jpg(&full_image_frame, 80, &_jpg_buf, &_jpg_buf_len)) {
            ESP_LOGE(TAG, "JPEG compression failed");
            xSemaphoreGive(full_image_mutex);
            return ESP_FAIL;
        }
        size_t hlen = snprintf((char *)part_buf,
                               128,
                               _STREAM_PART,
                               _jpg_buf_len,
                               full_image_frame.timestamp.tv_sec,
                               full_image_frame.timestamp.tv_usec);
        xSemaphoreGive(full_image_mutex);

        res = httpd_resp_send_chunk(req, _STREAM_BOUNDARY, strlen(_STREAM_BOUNDARY));
        if (res != ESP_OK) {
            return res;
        }

        res = httpd_resp_send_chunk(req, (const char *)part_buf, hlen);
        if (res != ESP_OK) {
            return res;
        }

        res = httpd_resp_send_chunk(req, (const char *)_jpg_buf, _jpg_buf_len);
        if (res != ESP_OK) {
            return res;
        }

        if (format != PIXFORMAT_JPEG) {
            free(_jpg_buf);
            _jpg_buf = NULL;
        }
    }

    return ESP_OK;
}

static esp_err_t cmd_handler(httpd_req_t *req)
{
    char *buf = NULL;
    char variable[32];
    char value[32];

    if (parse_get(req, &buf) != ESP_OK || httpd_query_key_value(buf, "var", variable, sizeof(variable)) != ESP_OK
        || httpd_query_key_value(buf, "val", value, sizeof(value)) != ESP_OK) {
        free(buf);
        httpd_resp_send_404(req);
        return ESP_FAIL;
    }
    free(buf);

    int val = atoi(value);
    ESP_LOGI(TAG, "%s = %d", variable, val);
    sensor_t *s = esp_camera_sensor_get();
    int res = 0;

    if (!strcmp(variable, "quality"))
        res = s->set_quality(s, val);
    else if (!strcmp(variable, "aec"))
        res = s->set_exposure_ctrl(s, val);
    else if (!strcmp(variable, "ae_level"))
        res = s->set_ae_level(s, val);
    else if (!strcmp(variable, "aec_value"))
        res = s->set_aec_value(s, val);
    else if (!strcmp(variable, "agc_gain"))
        res = s->set_agc_gain(s, val);
    else if (!strcmp(variable, "lenc"))
        res = s->set_lenc(s, val);
    else if (!strcmp(variable, "hmirror"))
        res = s->set_hmirror(s, val);
    else if (!strcmp(variable, "vflip"))
        res = s->set_vflip(s, val);
    else
        res = -1;

    if (res) {
        return httpd_resp_send_500(req);
    }

    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    return httpd_resp_send(req, NULL, 0);
}

static esp_err_t status_handler(httpd_req_t *req)
{
    static char json_response[1024];

    sensor_t *s = esp_camera_sensor_get();
    char *p = json_response;
    *p++ = '{';
    p += sprintf(p, "\"quality\":%u,", s->status.quality);
    p += sprintf(p, "\"aec\":%u,", s->status.aec);
    p += sprintf(p, "\"ae_level\":%d,", s->status.ae_level);
    p += sprintf(p, "\"aec_value\":%u,", s->status.aec_value);
    p += sprintf(p, "\"agc_gain\":%u,", s->status.agc_gain);
    p += sprintf(p, "\"lenc\":%u,", s->status.lenc);
    p += sprintf(p, "\"hmirror\":%u,", s->status.hmirror);
    p += sprintf(p, "\"vflip\":%u", s->status.vflip);
    *p++ = '}';
    *p++ = 0;
    httpd_resp_set_type(req, "application/json");
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    return httpd_resp_send(req, json_response, strlen(json_response));
}

static esp_err_t supervisor_command_handler(httpd_req_t *req)
{
    char *buf = NULL;
    char command[32];
    char motors_continuous[32];

    if (parse_get(req, &buf) != ESP_OK || httpd_query_key_value(buf, "cmd", command, sizeof(command)) != ESP_OK
        || httpd_query_key_value(buf, "continuous", motors_continuous, sizeof(motors_continuous)) != ESP_OK) {
        free(buf);
        httpd_resp_send_404(req);
        return ESP_FAIL;
    }
    free(buf);

    ESP_LOGI(TAG, "supervisor_command_handler : %s (continous:%s)", command, motors_continuous);
    if (!strcmp(command, "start-tracking")) {
        // TODO
    } else if (!strcmp(command, "stop")) {
        supervisor_stop();
    } else {
        motors_direction_t direction = motors_direction_t::NONE;
        if (!strcmp(command, "up"))
            direction = motors_direction_t::UP;
        else if (!strcmp(command, "up-right"))
            direction = motors_direction_t::UP_RIGHT;
        else if (!strcmp(command, "right"))
            direction = motors_direction_t::RIGHT;
        else if (!strcmp(command, "down-right"))
            direction = motors_direction_t::DOWN_RIGHT;
        else if (!strcmp(command, "down"))
            direction = motors_direction_t::DOWN;
        else if (!strcmp(command, "down-left"))
            direction = motors_direction_t::DOWN_LEFT;
        else if (!strcmp(command, "left"))
            direction = motors_direction_t::LEFT;
        else if (!strcmp(command, "up-left"))
            direction = motors_direction_t::UP_LEFT;
        else {
            ESP_LOGE(TAG, "Incorrect direction");
            httpd_resp_send_404(req);
            return ESP_FAIL;
        }
        if (!strcmp(motors_continuous, "1")) {
            supervisor_start_manual_move_continuous(direction);
        } else {
            supervisor_start_manual_move_one_step(direction);
        }
    }

    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    return httpd_resp_send(req, NULL, 0);
}

static esp_err_t supervisor_status_handler(httpd_req_t *req)
{
    auto supervisor_state = supervisor_get_state();
    ESP_LOGI(TAG, "motors supervisor_state = %s", supervisor_state);
    auto motors_state = motors_get_state();
    ESP_LOGI(TAG, "motors state = %s", motors_state);

    static char json_response[1024];
    sprintf(json_response, "{\"supervisor-state\":\"%s\", \"motors-state\":\"%s\"}", supervisor_state, motors_state);
    httpd_resp_set_type(req, "application/json");
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    return httpd_resp_send(req, json_response, strlen(json_response));
}

static esp_err_t index_handler(httpd_req_t *req)
{
    extern const unsigned char index_ov2640_html_gz_start[] asm("_binary_index_ov2640_html_gz_start");
    extern const unsigned char index_ov2640_html_gz_end[] asm("_binary_index_ov2640_html_gz_end");
    size_t index_ov2640_html_gz_len = index_ov2640_html_gz_end - index_ov2640_html_gz_start;

    httpd_resp_set_type(req, "text/html");
    httpd_resp_set_hdr(req, "Content-Encoding", "gzip");
    sensor_t *s = esp_camera_sensor_get();
    if (s == NULL) {
        ESP_LOGE(TAG, "Camera sensor not found");
        return httpd_resp_send_500(req);
    }
    if (s->id.PID != OV2640_PID) {
        ESP_LOGE(TAG, "Camera sensor not supported (only OV2640 is supported)");
        return httpd_resp_send_500(req);
    }
    return httpd_resp_send(req, (const char *)index_ov2640_html_gz_start, index_ov2640_html_gz_len);
}

void register_httpd(const QueueHandle_t frame_i, const QueueHandle_t frame_o, const bool return_fb)
{
    xQueueFrameI = frame_i;
    xQueueFrameO = frame_o;
    gReturnFB = return_fb;

    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.max_uri_handlers = 12;
    config.lru_purge_enable = true;

    httpd_uri_t index_uri = {.uri = "/", .method = HTTP_GET, .handler = index_handler, .user_ctx = NULL};

    httpd_uri_t status_uri = {.uri = "/status", .method = HTTP_GET, .handler = status_handler, .user_ctx = NULL};

    httpd_uri_t cmd_uri = {.uri = "/control", .method = HTTP_GET, .handler = cmd_handler, .user_ctx = NULL};

    httpd_uri_t capture_uri = {.uri = "/capture", .method = HTTP_GET, .handler = capture_handler, .user_ctx = NULL};

    httpd_uri_t capture_area_uri = {
        .uri = "/capture_area", .method = HTTP_GET, .handler = capture_area_handler, .user_ctx = NULL};

    httpd_uri_t stream_uri = {.uri = "/stream", .method = HTTP_GET, .handler = stream_handler, .user_ctx = NULL};

    httpd_uri_t supervisor_command_uri = {
        .uri = "/supervisor_command", .method = HTTP_GET, .handler = supervisor_command_handler, .user_ctx = NULL};

    httpd_uri_t supervisor_status_uri = {
        .uri = "/supervisor_status", .method = HTTP_GET, .handler = supervisor_status_handler, .user_ctx = NULL};

    full_image_mutex = xSemaphoreCreateMutex();
    full_image_ready = xSemaphoreCreateBinary();
    sun_tracker_register_full_image_callback(full_image_updated);

    ESP_LOGI(TAG, "Starting web server on port: '%d'", config.server_port);
    if (httpd_start(&camera_httpd, &config) == ESP_OK) {
        httpd_register_uri_handler(camera_httpd, &index_uri);
        httpd_register_uri_handler(camera_httpd, &cmd_uri);
        httpd_register_uri_handler(camera_httpd, &status_uri);
        httpd_register_uri_handler(camera_httpd, &capture_uri);
        httpd_register_uri_handler(camera_httpd, &capture_area_uri);
        httpd_register_uri_handler(camera_httpd, &supervisor_command_uri);
        httpd_register_uri_handler(camera_httpd, &supervisor_status_uri);
    }

    config.server_port += 1;
    config.ctrl_port += 1;
    ESP_LOGI(TAG, "Starting stream server on port: '%d'", config.server_port);
    if (httpd_start(&stream_httpd, &config) == ESP_OK) {
        httpd_register_uri_handler(stream_httpd, &stream_uri);
    }
}
