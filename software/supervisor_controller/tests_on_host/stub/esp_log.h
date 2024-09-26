// Copyright (C) 2024 Rémi Peuchot (https://remipch.github.io/)
// This code is distributed under GNU GPL v3 license

// For test purpose, define minimal stuff to print the equivalent
// of the few esp_log macros used from original esp component
// (advanced features like tag or level filtering are simply ignored)

#include <cstdio>

#define ERROR_PREFIX "\x1B[31mE"
#define WARNING_PREFIX "\x1B[33mW"
#define INFO_PREFIX "\x1B[32mI"
#define DEBUG_PREFIX "D"
#define VERBOSE_PREFIX "\x1B[90mV"

#define PRINT_LOG(prefix, tag, format, ...) printf("  %s %s: " format "\033[0m\n", prefix, tag, ##__VA_ARGS__)

#define ESP_LOGE(tag, format, ...) PRINT_LOG(ERROR_PREFIX, tag, format, ##__VA_ARGS__)
#define ESP_LOGW(tag, format, ...) PRINT_LOG(WARNING_PREFIX, tag, format, ##__VA_ARGS__)
#define ESP_LOGI(tag, format, ...) PRINT_LOG(INFO_PREFIX, tag, format, ##__VA_ARGS__)
#define ESP_LOGD(tag, format, ...) PRINT_LOG(DEBUG_PREFIX, tag, format, ##__VA_ARGS__)
#define ESP_LOGV(tag, format, ...) PRINT_LOG(VERBOSE_PREFIX, tag, format, ##__VA_ARGS__)
