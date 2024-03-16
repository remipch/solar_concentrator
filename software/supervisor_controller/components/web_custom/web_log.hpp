#pragma once

// Max size of log text cached between calls to 'web_log_get_last'
// including terminating NULL char
// If web_log_get_last is not called frequently enough, last logs will be lost
const int LOG_BUFFER_SIZE = 1000;

void web_log_init();

// Buffer must be of size >= LOG_BUFFER_SIZE
void web_log_get_last(char *buffer);
