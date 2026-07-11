#include "logger.h"
#include <stdarg.h>
#include <stdio.h>
#include <time.h>

FILE *log_file = NULL;

bool init_logger(const char *log_path) {
  log_file = fopen(log_path, "w");
  // disable buffering
  setvbuf(log_file, NULL, _IONBF, 0);
  return true;
}

void logger_log(enum log_status status, const char *func, const char *fmt, ...) {
  char buffer[LOG_MAX_BUFFER_SIZE];
  char time_buffer[LOG_MAX_TIME_BUFFER_SIZE];
  time_t now;
  struct tm *tm;
  va_list args;

  now = time(NULL);
  tm = localtime(&now);
  va_start(args, fmt);
  vsnprintf(buffer, sizeof(buffer), fmt, args);
  strftime(time_buffer, sizeof(time_buffer), "%F %X", tm);
  switch (status) {
  case LOG:
    fprintf(log_file, COLOR_START(GREEN) "[LOG @ %s]" COLOR_START(GREEN) " %s:" COLOR_END " %s\n",
            time_buffer, func, buffer);
    break;
  case WARNING:
    fprintf(log_file, COLOR_START(GREEN) "[WARNING @ %s]" COLOR_START(YELLOW) " %s:" COLOR_END " %s\n",
            time_buffer, func, buffer);
    break;
  case FATAL:
    fprintf(log_file, COLOR_START(GREEN) "[FATAL @ %s]" COLOR_START(RED) " %s:" COLOR_END " %s\n",
            time_buffer, func, buffer);
    break;
  }
  va_end(args);
}