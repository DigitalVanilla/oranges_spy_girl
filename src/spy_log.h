#ifndef SPY_GIRL_LOG_H
#define SPY_GIRL_LOG_H

#include <stdarg.h>

void SpyLogOpen(const char *filename);
void SpyLogClose(void);
void SpyLog(const char *format, ...);
void SpyLogV(const char *format, va_list args);

#endif
