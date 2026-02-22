#include <string.h>
#include <stdio.h>

#include "vmf.h"

int get_id(KV_Pair *pair) {
    KV_Pair *id = KV_FindPairOfType(pair, "id", KV_TYPE_STRING);
    if (id) {
        const char *str = KV_GetString(id);
        return strtol(str, NULL, 10);
    } // TODO: fatal
    return -1;
}

af_bool set_id(KV_Pair *pair, int id) {
    KV_Pair *id_v = KV_FindPairOfType(pair, "id", KV_TYPE_STRING);
    if (id_v) {
        char buf[32];
        snprintf(buf, sizeof(buf), "%d", id);
        KV_SetString(id_v, buf);
        return AF_TRUE;
    } // TODO: fatal
    return AF_FALSE;
}

af_bool dump_plane(char *buffer, size_t bufsize, const vec3x3_t *plane) {
    int n = snprintf(buffer, bufsize,
        "(%g %g %g) (%g %g %g) (%g %g %g)",
        plane->v[0].x, plane->v[0].y, plane->v[0].z,
        plane->v[1].x, plane->v[1].y, plane->v[1].z,
        plane->v[2].x, plane->v[2].y, plane->v[2].z
    );

#ifdef AF_DEBUG
    log("dump_plane dumped len %d\n", n);
#endif

    if (n < 0 || (size_t)n >= bufsize) {
        return AF_FALSE;
    }

    return AF_TRUE;
}

af_bool parse_plane(const char *plane, vec3x3_t *v) {
    if (sscanf(plane, "(%lf %lf %lf) (%lf %lf %lf) (%lf %lf %lf)",
        &v->v[0].x, &v->v[0].y, &v->v[0].z,
        &v->v[1].x, &v->v[1].y, &v->v[1].z,
        &v->v[2].x, &v->v[2].y, &v->v[2].z
    ) != 9) {
        fprintf(stderr, "Error parsing plane\n");
        return AF_FALSE;
    }

    return AF_TRUE;
}

af_bool parse_vert(const char *plane, vec3_t *v) {
    return sscanf(plane, "%lf %lf %lf", &v->x, &v->y, &v->z) == 3;
}

KV_Pair *get_face_by_id(KV_Pair *solid, int id) {
    int i = 0;
    KV_Pair *sub;
    while ((sub = KV_GetPair(solid, i++))) {
        if (KV_GetDataType(sub) == KV_TYPE_NONE && !strcmp(KV_GetKey(sub), "side")) {
            if (get_id(sub) == id) {
                return sub;
            }
        }
    }

    return NULL;
}

KV_Pair *get_solid_by_id(KV_Pair *entity, int id) {
    int i = 0;
    KV_Pair *sub;
    while ((sub = KV_GetPair(entity, i++))) {
        if (KV_GetDataType(sub) == KV_TYPE_NONE && !strcmp(KV_GetKey(sub), "solid")) {
            if (get_id(sub) == id) {
                return sub;
            }
        }
    }

    return NULL;
}
