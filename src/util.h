#ifndef UTIL_H
#define UTIL_H

#include <stdarg.h>

#include "anglefix.h"

#define OOM "Out of memory\n"

af_bool is_zero(double x);
af_bool file_exists(const char *path);
af_bool vfprintf_color(FILE *stream, int color, const char *fmt, va_list args);
void warn(const char *fmt, ...);
void verr(const char *fmt, va_list args);
void err(const char *fmt, ...);
void fatal(const char *fmt, ...);
af_bool insert_prefix_before_ext(const char *fullpath, const char *prefix, char *out, size_t out_size);

#endif /* UTIL_H */
