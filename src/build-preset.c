#include <stdio.h>

#include <keyvalues.h>

#include "build-preset.h"
#include "util.h"

build_step_t build_steps[] = {
    { NULL,             "1", "anglefix.exe", "$path\\$file.vmf" },
    { NULL,             "1", "$bsp_exe",     "-game $gamedir $path\\$file-anglefixed" },
    { NULL,             "1", "$vis_exe",     "-game $gamedir $path\\$file-anglefixed" },
    { NULL,             "1", "$light_exe",   "-game $gamedir $path\\$file-anglefixed" },
    { SPECIAL_CMD_COPY, "1", NULL,           "$path\\$file-anglefixed.bsp $path\\$file.bsp" },
    { SPECIAL_CMD_COPY, "1", NULL,           "$path\\$file.bsp $bspdir\\$file.bsp" },
    { NULL,             "0", "$game_exe",    "-dev -console -allowdebug -hijack -game $gamedir +map $file-anglefixed" },
};

const char CFG[] = "hammerplusplus\\hammerplusplus_sequences.cfg";

void add_build_preset() {
    if (!file_exists(CFG)) {
        return;
    }

    KV_Pair *cfg = KV_ParseFile(CFG);
    if (cfg) {
        KV_Pair *presets = KV_FindPairOfType(cfg, "Command Sequences", KV_TYPE_NONE);
        if (presets) {
            KV_Pair *only = KV_FindPairOfType(presets, "Anglefix", KV_TYPE_NONE);
            if (!only) {
                KV_Pair *anglefix = KV_NewList("Anglefix");
                for (int i = 0; i < sizeof(build_steps) / sizeof(build_steps[0]); i++) {
                    const build_step_t *step = &build_steps[i];
                    char n[3];
                    snprintf(n, sizeof(n), "%d", i);
                    KV_Pair *values = KV_NewList(n);
                    KV_AddTail(values, KV_NewString("enable", step->enable));
                    if (step->specialcmd) {
                        KV_AddTail(values, KV_NewString("specialcmd", step->specialcmd));
                    } else {
                        KV_AddTail(values, KV_NewString("specialcmd", "0"));
                        KV_AddTail(values, KV_NewString("run", step->run));
                    }
                    KV_AddTail(values, KV_NewString("parms", step->parms));
                    KV_AddTail(anglefix, values);
                }

                KV_AddTail(presets, anglefix);

                warn("first run: added \"Anglefix\" hammer++ build preset\n");
                KV_Save(cfg, CFG);
            }
        }

        KV_PairDestroy(cfg);
    }
}
