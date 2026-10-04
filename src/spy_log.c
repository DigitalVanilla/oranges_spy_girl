#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "spy_log.h"

#define SPY_LOG_BUFFER_SIZE 1024

static FILE *log_file = NULL;

void SpyLogOpen(const char *filename)
{
  if (log_file != NULL) {
    return;
  }

  log_file = fopen(filename, "w");
  if (log_file != NULL) {
    setvbuf(log_file, NULL, _IONBF, 0);
  }
}

void SpyLogClose(void)
{
  if (log_file != NULL) {
    fflush(log_file);
    fclose(log_file);
    log_file = NULL;
  }
}

void SpyLogV(const char *format, va_list args)
{
  char buffer[SPY_LOG_BUFFER_SIZE];
  int written;

  written = vsnprintf(buffer, sizeof(buffer), format, args);
  if (written < 0) {
    strcpy(buffer, "[log format error]\n");
  } else {
    buffer[sizeof(buffer) - 1] = 0;
  }

  printf("%s", buffer);
  fflush(stdout);

  if (log_file != NULL) {
    fputs(buffer, log_file);
    fflush(log_file);
  }
}

void SpyLog(const char *format, ...)
{
  va_list args;

  va_start(args, format);
  SpyLogV(format, args);
  va_end(args);
}
