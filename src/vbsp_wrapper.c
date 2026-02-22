#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <process.h>
#define SPAWN _spawnv
#define WAIT_MODE _P_WAIT
#include <io.h>

#define MAX_PATH_LEN 4096
#define CMD_BUF 8192
#define REAL_EXE "real_vbspplusplus"
#define ANGLEFIX_EXE "anglefix.exe"
#define NAME "vbspplusplus_wrapper"

// mostly chatgpt code

static int replace_extension(
    const char *path,
    const char *replacement,
    char *out,
    size_t out_size
) {
    const char *dot = strrchr(path, '.');
    size_t path_len = strlen(path);
    size_t repl_len = strlen(replacement);

    if (dot) {
        size_t base_len = (size_t)(dot - path);
        if (base_len + repl_len + 1 > out_size) return 0;
        memcpy(out, path, base_len);
        memcpy(out + base_len, replacement, repl_len + 1);
    } else {
        /* No extension: append replacement */
        if (path_len + repl_len + 1 > out_size) return 0;
        memcpy(out, path, path_len);
        memcpy(out + path_len, replacement, repl_len + 1);
    }

    return 1;
}

static void rename_output(const char *path, const char *ext) {
    char from[MAX_PATH_LEN];
    char to[MAX_PATH_LEN];

    if (!replace_extension(path, ext, to, sizeof(to))) {
        return;
    }

    if (!replace_extension(path, "-anglefixed", from, sizeof(from))) {
        return;
    }

    strncat(from, ext, sizeof(from) - strlen(from) - 1);

    rename(from, to);
}

static void append_quoted(char *dst, size_t dst_size, const char *arg) {
    strncat(dst, " \"", dst_size - strlen(dst) - 1);
    strncat(dst, arg, dst_size - strlen(dst) - 1);
    strncat(dst, "\"", dst_size - strlen(dst) - 1);
}

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "%s usage: %s [args...] <vmf>\n", NAME, argv[0]);
        return 1;
    }

    const char *input_path = argv[argc - 1];
    char anglefixed_vmf[MAX_PATH_LEN];
    char vmf[MAX_PATH_LEN];

    if (!replace_extension(input_path, ".vmf", vmf, sizeof(vmf))) {
        fprintf(stderr, "%s: path too long\n", NAME);
        return 1;
    }

    if (!replace_extension(input_path, "-anglefixed", anglefixed_vmf, sizeof(anglefixed_vmf))) {
        fprintf(stderr, "%s: path too long\n", NAME);
        return 1;
    }

    const char *anglefix_args[] = {
        ANGLEFIX_EXE,
        vmf,
        NULL
    };

    printf("%s: running %s\n", NAME, ANGLEFIX_EXE);
    if (SPAWN(WAIT_MODE, ANGLEFIX_EXE, anglefix_args) != 0) {
        fprintf(stderr, "%s: %s failed\n", NAME, ANGLEFIX_EXE);
        return 1;
    }

    printf("%s: running %s with anglefixed vmf\n", NAME, REAL_EXE);
    // couldn't figure out why vbsp thought the -game path wasn't valid when using _spawnv
    // so for now, use system()
    char cmd[CMD_BUF];
    snprintf(cmd, sizeof(cmd), REAL_EXE);
    for (int i = 1; i < argc; ++i) {
        if (i == argc - 1) {
            append_quoted(cmd, sizeof(cmd), anglefixed_vmf);
        } else {
            append_quoted(cmd, sizeof(cmd), argv[i]);
        }
    }

    int ret = system(cmd);

    // if (ret != 0) {
    //     fprintf(stderr, "%s failed (%d)\n", REAL_EXE, ret);
    //     return 1;
    // }

    /* ---- rename outputs ---- */
    printf("%s: renaming anglefixed .bsp, .log, .prt\n", NAME);
    rename_output(input_path, ".bsp");
    rename_output(input_path, ".log");
    rename_output(input_path, ".prt");

    return 0;
}
