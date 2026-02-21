#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

#include "util.h"

af_bool is_zero(double x) {
    const double eps = 1e-5f; // very arbitrary
    return fabsl(x) < eps ? AF_TRUE : AF_FALSE;
}

af_bool file_exists(const char *path) {
    FILE *f = fopen(path, "rb");
    if (f) {
        fclose(f);
        return AF_TRUE;
    }
    return AF_FALSE;
}

af_bool vfprintf_color(FILE *stream, int color, const char *fmt, va_list args) {
    if (stream == stdout || stream == stderr) {
#ifdef _WIN32
        HANDLE hConsole = (stream == stdout)
            ? GetStdHandle(STD_OUTPUT_HANDLE)
            : GetStdHandle(STD_ERROR_HANDLE);

        CONSOLE_SCREEN_BUFFER_INFO consoleInfo;
        GetConsoleScreenBufferInfo(hConsole, &consoleInfo);
        WORD saved_attributes = consoleInfo.wAttributes;

        SetConsoleTextAttribute(hConsole, color);
#else
        // TODO: linux color
#endif
        vfprintf(stream, fmt, args);
#ifdef _WIN32
        SetConsoleTextAttribute(hConsole, saved_attributes);
#endif

        return AF_TRUE;
    }

    return AF_FALSE;
}

#if _WIN32
#define YELLOW (FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY)
#endif
void warn(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
#if _WIN32
    vfprintf_color(stderr, YELLOW, fmt, args);
#else
    vfprintf_color(stderr, 0, fmt, args);
#endif
    va_end(args);
}

#if _WIN32
#define RED (FOREGROUND_RED | FOREGROUND_INTENSITY)
#endif
void verr(const char *fmt, va_list args) {
#if _WIN32
    vfprintf_color(stderr, RED, fmt, args);
#else
    vfprintf_color(stderr, 0, fmt, args);
#endif
}

void err(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    verr(fmt, args);
    va_end(args);
}

void fatal(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    verr(fmt, args);
    va_end(args);
    exit(1);
}

af_bool insert_prefix_before_ext(const char *fullpath, const char *prefix, char *out, size_t out_size) {
    const char *slash1 = strrchr(fullpath, '/');
    const char *slash2 = strrchr(fullpath, '\\');
    const char *slash = (slash1 > slash2) ? slash1 : slash2; // pick the rightmost slash
    const char *filename = (slash) ? slash + 1 : fullpath; // filename only
    const char *dot = strrchr(filename, '.'); // last dot in filename
    size_t base_len;

    if (dot) {
        base_len = dot - filename; // length of base filename
    } else {
        base_len = strlen(filename); // no extension
        dot = filename + base_len;
    }

    size_t dir_len = (slash) ? (slash - fullpath + 1) : 0; // include trailing '/'
    size_t needed = dir_len + base_len + strlen(prefix) + strlen(dot) + 1;

    if (needed > out_size) return AF_FALSE; // not enough space

    // Copy directory part
    if (dir_len > 0) {
        memcpy(out, fullpath, dir_len);
    }

    // Copy base filename
    memcpy(out + dir_len, filename, base_len);
    // Copy prefix
    strcpy(out + dir_len + base_len, prefix);
    // Copy extension
    strcpy(out + dir_len + base_len + strlen(prefix), dot);

    return AF_TRUE;
}
