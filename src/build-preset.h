#ifndef BUILD_PRESET_H
#define BUILD_PRESET_H

#define SPECIAL_CMD_COPY "257"
#define SPECIAL_CMD_DELETE "258"
#define SPECIAL_CMD_RENAME "259"

typedef struct {
    const char *specialcmd;
    const char *enable;
    const char *run;
    const char *parms;
} build_step_t;

extern const char CFG[];

void add_build_preset();

#endif /* BUILD_PRESET_H */
