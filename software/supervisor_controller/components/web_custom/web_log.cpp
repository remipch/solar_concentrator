#include "web_log.hpp"

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

#include <string.h>

static char log_buffer[LOG_BUFFER_SIZE];
static char *log_buffer_end; // Points to the terminating NULL char

static SemaphoreHandle_t buffer_mutex;

static const int MUTEX_TIMEOUT_MS = 100;

#if CONFIG_LOG_COLORS == 1
static const char *WARNING_PREFIX = "\033[0;33mW";
static const char *ERROR_PREFIX = "\033[0;31mE";
#else
static const char *WARNING_PREFIX = "W";
static const char *ERROR_PREFIX = "E";
#endif

bool starts_with(const char *str, const char *prefix) { return strncmp(str, prefix, strlen(prefix)) == 0; }

// Called when a LOG_ macro has been called
int web_log_update(const char *format, va_list args)
{
    assert(xSemaphoreTake(buffer_mutex, pdMS_TO_TICKS(MUTEX_TIMEOUT_MS)));

    // Always print in console (the default log callback is replaced by this one)
    int n = vprintf(format, args);

    // Cache the log if this is a warning or an error
    if (starts_with(format, ERROR_PREFIX) || starts_with(format, WARNING_PREFIX)) {
        int available_len = LOG_BUFFER_SIZE - (log_buffer_end - log_buffer);
        int writen_len = vsnprintf(log_buffer_end, available_len, format, args);

        // Note: 'available_len' counts the terminating NULL but 'writen_len' does not
        if (writen_len < available_len) {
            log_buffer_end += writen_len;
        } else {
            *log_buffer_end = '\0';
            log_buffer_end = log_buffer + LOG_BUFFER_SIZE - 1;
        }
    }

    xSemaphoreGive(buffer_mutex);
    return n;
};

// Must be called within mutex protection
void web_log_reset()
{
    log_buffer[0] = '\0';
    log_buffer_end = log_buffer;
};

void web_log_init()
{
    buffer_mutex = xSemaphoreCreateMutex();
    web_log_reset();
    esp_log_set_vprintf(web_log_update);
}

void web_log_get_last(char *buffer)
{
    assert(xSemaphoreTake(buffer_mutex, pdMS_TO_TICKS(MUTEX_TIMEOUT_MS)));
    strcpy(buffer, log_buffer);
    web_log_reset();
    xSemaphoreGive(buffer_mutex);
}
