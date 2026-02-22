#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

#include "util.h"

#ifdef _WIN32
#define YELLOW (FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY)
#define RED (FOREGROUND_RED | FOREGROUND_INTENSITY)
#else
#define YELLOW 0
#define RED 0
#endif

char *log_file = NULL;

void set_log_file(char *path) {
    log_file = path;
}

af_bool is_near_zero(double x) {
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

void vfprintf_color(FILE *stream, int color, const char *fmt, va_list args) {
    if ((stream == stdout || stream == stderr) && color) {
#ifdef _WIN32
        HANDLE hConsole = (stream == stdout)
            ? GetStdHandle(STD_OUTPUT_HANDLE)
            : GetStdHandle(STD_ERROR_HANDLE);

        CONSOLE_SCREEN_BUFFER_INFO consoleInfo;
        GetConsoleScreenBufferInfo(hConsole, &consoleInfo);
        WORD saved_attributes = consoleInfo.wAttributes;

        SetConsoleTextAttribute(hConsole, color);
        vfprintf(stream, fmt, args);
        SetConsoleTextAttribute(hConsole, saved_attributes);
#else
        // TODO: linux color
        vfprintf(stream, fmt, args);
#endif
    } else {
        vfprintf(stream, fmt, args);
    }

    printf("write log?\n");
    if (log_file) {
        FILE *f = fopen(log_file, "a");
        if (f) {
            vfprintf(f, fmt, args);
            fclose(f);
        }
    }
}

void af_warn(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vfprintf_color(stderr, YELLOW, fmt, args);
    va_end(args);
}

void af_log(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vfprintf_color(stdout, 0, fmt, args);
    va_end(args);
}

void verr(const char *fmt, va_list args) {
    vfprintf_color(stderr, RED, fmt, args);
}

void af_err(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    verr(fmt, args);
    va_end(args);
}

void af_fatal(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    verr(fmt, args);
    va_end(args);
    exit(1);
}

// chatgpt
char *replace_extension(const char *path, const char *new_ext) {
    const char *dot;
    const char *slash;
    size_t base_len;
    size_t ext_len;
    char *result;

    if (!path || !new_ext)
        return NULL;

    slash = strrchr(path, '/');
    dot = strrchr(path, '.');

    /* Dot must be after the last slash to count as an extension */
    if (!dot || (slash && dot < slash)) {
        base_len = strlen(path);
    } else {
        base_len = (size_t)(dot - path);
    }

    ext_len = strlen(new_ext);

    result = malloc(base_len + ext_len + 1);
    if (!result)
        return NULL;

    memcpy(result, path, base_len);
    memcpy(result + base_len, new_ext, ext_len);
    result[base_len + ext_len] = '\0';

    return result;
}
