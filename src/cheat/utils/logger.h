#pragma once
#define LOG_MAX_BUFFER_SIZE 1024
#define LOG_MAX_TIME_BUFFER_SIZE 128
#define COLOR_START(x) "\033[" x "m"
#define COLOR_END "\033[0m"
#define RED "31"
#define GREEN "32"
#define YELLOW "33"
#define LOGGER_LOG(status, fmt, ...) logger_log(status, __func__, fmt __VA_OPT__(, ) __VA_ARGS__)

enum log_status { LOG, WARNING, FATAL };

bool init_logger(const char *log_path);
void logger_log(enum log_status status, const char *func, const char *fmt, ...);