#ifndef UTIL_H
#define UTIL_H

#include <stdarg.h>

#include "anglefix.h"

#define OOM "Out of memory\n"

void set_log_file(char *path);
af_bool is_near_zero(double x);
af_bool file_exists(const char *path);
void vfprintf_color(FILE *stream, int color, const char *fmt, va_list args);
void af_log(const char *fmt, ...);
void af_warn(const char *fmt, ...);
void verr(const char *fmt, va_list args);
void af_err(const char *fmt, ...);
void af_fatal(const char *fmt, ...);
char *replace_extension(const char *path, const char *new_ext);

#endif /* UTIL_H */
