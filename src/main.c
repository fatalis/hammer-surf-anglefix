#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <stdint.h>
#include <math.h>
#include <assert.h>
#include <float.h>
#include <time.h>

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
            af_fatal("usage: anglefix.exe [-debug] <path>\n");
        }
    }

    if (!path) {
        af_fatal("usage: anglefix.exe [-debug] <path>\n");
    }

    char *log_file = replace_extension(path, "-anglefixed.log");
    set_log_file(log_file);

    // mimic the way valve tools do this
    af_log("==================================================================\n");
    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);
    char buf[32];
    strftime(buf, sizeof(buf), "Date: %Y-%m-%d %H:%M:%S\n", tm_info);
    af_log(buf);
    for (int i = 0; i < argc; i++) {
        af_log("%s ", argv[i]);
    }
    af_log("\n");
    af_log("==================================================================\n");

    char *vmf_file = replace_extension(path, "-anglefixed.vmf");
    char *collision_file = replace_extension(path, "-anglefixed-collision.vmf");

    char *output_collision_only = NULL;
    char *output = anglefix_generate_output(path, &output_collision_only);

    FILE *f = fopen(vmf_file, "w");
    if (!f) {
        af_fatal("Cannot open output file for writing :%s\n", vmf_file);
    }
    fputs(output, f);
    fclose(f);
    af_log("wrote %s\n", vmf_file);

    f = fopen(collision_file, "w");
    if (!f) {
        af_fatal("Cannot open output file for writing: %s\n", collision_file);
    }
    fputs(output_collision_only, f);
    fclose(f);
    af_log("wrote %s\n", collision_file);

    free(vmf_file);
    free(collision_file);
    free(log_file);
    KV_free(output);
    KV_free(output_collision_only);

    return 0;
}
