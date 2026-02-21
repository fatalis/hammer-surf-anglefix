#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <stdint.h>
#include <math.h>
#include <assert.h>
#include <float.h>

#include <keyvalues.h>

#include "anglefix.h"
#include "util.h"
#include "anglefixer.h"
#include "build-preset.h"

// TODO: if none of the faces of an ent need anglefix, warn user?
// TODO: figure out why uv is being modified
// TODO: log to the same log as vbsp
// TODO: detect if user mistakenly put the flag on all faces

int main(int argc, const char *argv[]) {
    const char *path = NULL;

    add_build_preset();

    for (int i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "-debug")) {
            debug_mode = AF_TRUE;
        } else if (!path) {
            path = argv[i];
        } else {
            fatal("usage: anglefix.exe [-debug] <path>\n");
        }
    }

    if (!path) {
        fatal("usage: anglefix.exe [-debug] <path>\n");
    }

    char *output_collision_only = NULL;
    char *output = anglefix_generate_output(path, &output_collision_only);
    char new_path[MAX_PATH];

    if (insert_prefix_before_ext(path, "-anglefixed", new_path, sizeof(new_path))) {
        FILE *file = fopen(new_path, "w");
        if (!file) {
            KV_free(output);
            fatal("Cannot open output file for writing\n");
        }
        fputs(output, file);
        fclose(file);
        printf("wrote %s\n", new_path);
    } else {
        KV_free(output);
        fatal("-anglefixed.vmf: path too long\n");
    }

    if (insert_prefix_before_ext(path, "-anglefixed-collision", new_path, sizeof(new_path))) {
        FILE *file = fopen(new_path, "w");
        if (!file) {
            KV_free(output_collision_only);
            fatal("Cannot open output file for writing\n");
        }
        fputs(output_collision_only, file);
        fclose(file);
        printf("wrote %s\n", new_path);
    } else {
        KV_free(output_collision_only);
        fatal("-anglefixed-collision.vmf: path too long\n");
    }

    KV_free(output);
    KV_free(output_collision_only);

    return 0;
}
