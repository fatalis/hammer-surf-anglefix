#include <stdio.h>
#include <assert.h>
#include <stdint.h>
#include <math.h>
#include <string.h>

#include "util.h"
#include "vmf.h"
#include "anglefixer.h"

static int faces_fixed = 0;
static int solids_fixed = 0;
static int solid_faces_fixed = 0;
static int solid_id_max = 0;
static int side_id_max = 0;
static int entity_id_max = 0;
static af_bool has_watermark = AF_FALSE;
af_bool debug_mode = AF_FALSE;

void displace_plane(KV_Pair *face, const vec3_t *displacement) {
    KV_Pair *plane_val = KV_FindPairOfType(face, "plane", KV_TYPE_STRING);
    if (plane_val) { // TODO: else fatal
       const char *plane_str = KV_GetString(plane_val);
       vec3x3_t plane;

       if (parse_plane(plane_str, &plane)) { // TODO: else fatal
            for (int i = 0; i < 3; i++) {
                plane.v[i].x += displacement->x;
                plane.v[i].y += displacement->y;
                plane.v[i].z += displacement->z;
            }

            char str[BUF_SIZE_PLANE];
            dump_plane(str, sizeof(str), &plane);
            KV_SetString(plane_val, str);
       }
    }
}

void displace_verts(KV_Pair *face, const vec3_t *displacement) {
    KV_Pair *verts = KV_FindPairOfType(face, "vertices_plus", KV_TYPE_NONE);
    if (verts) {
        KV_Pair *vert_v;
        int i = 0;
        char str[BUF_SIZE_VERT];

        while ((vert_v = KV_GetPair(verts, i++))) {
            if (!strcmp(KV_GetKey(vert_v), "v")) {
                if (KV_GetDataType(vert_v) == KV_TYPE_STRING) {
                    char *vert_str = KV_GetString(vert_v);
                    vec3_t vert;
                    if (parse_vert(vert_str, &vert)) {
                        vert.x += displacement->x;
                        vert.y += displacement->y;
                        vert.z += displacement->z;
                        snprintf(str, sizeof(str), "%g %g %g", vert.x, vert.y, vert.z);
                        KV_SetString(vert_v, str);
                        /* printf("displaced vert: %f - %f - %f\n", vert.x, vert.y, vert.z); */
                    }
                }
            }
        }
    }
}

af_bool get_face_displacement(KV_Pair *entity, KV_Pair *face, vec3_t *out, double *distance) {
    KV_Pair *plane_val = KV_FindPairOfType(face, "plane", KV_TYPE_STRING);

    if (plane_val) {
        const char *plane_str = KV_GetString(plane_val);
        vec3x3_t plane;

        if (parse_plane(plane_str, &plane)) {
            vec3_t normal;
            euler_t euler;
            plane_normal(&plane, &normal);
            vec_to_euler_angles(&normal, &euler);

            double theta = euler.yaw;
            double move_distance = calculate_move_distance(theta);

            vec3_t displacement = { 0.0, 0.0, 0.0 };
            int entity_id = get_id(entity);
            int face_id = get_id(face);
            if (!is_zero(move_distance)) {
                displacement.x = move_distance * cosl(theta);
                displacement.y = move_distance * sinl(theta);
                /* printf("brush %d face %d displaced by %g\n", entity_id, face_id, move_distance); */
                /* faces_fixed++; */
                *out = displacement;
                *distance = move_distance;
                return AF_TRUE;
            } else {
                printf("brush %d face %d no need to displace\n", entity_id, face_id);
                return AF_FALSE;
            }
        }
    }

    return AF_FALSE;
}

void remove_temp_flags(KV_Pair *face) {
    KV_Pair *sg_v = KV_FindPairOfType(face, "smoothing_groups", KV_TYPE_STRING);
    if (sg_v) {
        const char *sg = KV_GetString(sg_v);
        int64_t val = strtoll(sg, NULL, 10);
        val &= ~(SMOOTHING_GROUP_MOVE_SOLID_FACES | SMOOTHING_GROUP_MOVE_FACES | SMOOTHING_GROUP_DEBUG);

        char buf[32];
        snprintf(buf, sizeof(buf), "%lld", val);
        KV_SetString(sg_v, buf);
    }
}

void handle_solid(KV_Pair *entity, const char *classname, int entity_id, KV_Pair *changes, KV_Pair *solid, af_bool is_visual, KV_Pair **pentity_copy) {
    int solid_id = get_id(solid);
    if (solid_id > solid_id_max) {
        solid_id_max = solid_id;
    }

    // TODO: use dynamic array instead to avoid copies
    KV_Pair *move_faces = KV_NewList(NULL); // internal
    KV_Pair *move_solid_faces = KV_NewList(NULL); // internal

    int i = 0;
    KV_Pair *sub;
    while ((sub = KV_GetPair(solid, i++))) {
        if (KV_GetDataType(sub) == KV_TYPE_NONE && !strcmp(KV_GetKey(sub), "side")) {
            KV_Pair *face = sub;
            int face_id = get_id(face);
            if (face_id > side_id_max) {
                side_id_max = face_id;
            }

            KV_Pair *sg_v = KV_FindPairOfType(face, "smoothing_groups", KV_TYPE_STRING);
            if (sg_v) {
                const char *sg = KV_GetString(sg_v);
                int64_t val = strtoll(sg, NULL, 10);

                // here the flags are removed from the original, but doesn't fully remove from the copy
                if (val & SMOOTHING_GROUP_MOVE_SOLID_FACES) {
                    if (is_visual) {
                        KV_AddTail(move_solid_faces, KV_PairCopy(sub));
                    } else {
                        warn("WARNING: smoothing group 32 set on brush %d <%s> solid %d face %d but brush is solid, ignoring\n", entity_id, classname, solid_id, face_id);
                    }
                    val &= ~SMOOTHING_GROUP_MOVE_SOLID_FACES;
                }
                if (val & SMOOTHING_GROUP_MOVE_FACES) {
                    if (is_visual) {
                        KV_AddTail(move_faces, KV_PairCopy(sub));
                    } else {
                        warn("WARNING: smoothing group 31 set on brush %d <%s> solid %d face %d but brush is solid, ignoring\n", entity_id, classname, solid_id, face_id);
                    }
                    val &= ~SMOOTHING_GROUP_MOVE_FACES;
                }
                if (val & SMOOTHING_GROUP_DEBUG) {
                    debug_mode = AF_TRUE;
                    val &= ~SMOOTHING_GROUP_DEBUG;
                    warn("WARNING: brush %d <%s> solid %d face %d had flag 30 - enabling debug mode globally\n");
                }

                char buf[32];
                snprintf(buf, sizeof(buf), "%lld", val);
                KV_SetString(sg_v, buf);
            }
        }
    }

    size_t n_move_solid_faces = KV_GetNodeCount(move_solid_faces);
    size_t n_move_faces = KV_GetNodeCount(move_faces);

    if (n_move_solid_faces > 0 && n_move_faces > 0) {
        fatal("USER ERROR: brush %d <%s> solid %d had faces with both flag 31 and 32\n", entity_id, classname, solid_id);
    }

    if (n_move_solid_faces > 0) {
        if (n_move_solid_faces > 1) {
            fatal("USER ERROR: brush %d <%s> solid %d had multiple faces flagged 31\n", entity_id, classname, solid_id);
        }

        if (!*pentity_copy) {
            *pentity_copy = KV_PairCopy(entity);
        }
        KV_Pair *entity_copy = *pentity_copy;

        KV_Pair *target_face = KV_GetPair(move_solid_faces, 0);
        assert(target_face);

        vec3_t displacement;
        double move_distance;
        if (get_face_displacement(entity, target_face, &displacement, &move_distance)) {
            int faces_moved = 0;

            KV_Pair *solid_copy = get_solid_by_id(entity_copy, solid_id);
            assert(solid_copy);

            i = 0;
            while ((sub = KV_GetPair(solid_copy, i++))) {
                if (KV_GetDataType(sub) == KV_TYPE_NONE && !strcmp(KV_GetKey(sub), "side")) {
                    KV_Pair *face = sub;
                    displace_plane(face, &displacement);
                    displace_verts(face, &displacement);
                    faces_moved++;
                    solid_faces_fixed++;
                }
            }

            solids_fixed++;
            printf("31: brush %d <%s> solid %d moved %d faces by %g\n", entity_id, classname, solid_id, faces_moved, move_distance);
        }
    } else if (n_move_faces > 0) {
        if (!*pentity_copy) {
            *pentity_copy = KV_PairCopy(entity);
        }
        KV_Pair *entity_copy = *pentity_copy;

        i = 0;
        KV_Pair *face;
        while ((face = KV_GetPair(move_faces, i++))) {
            vec3_t displacement;
            int face_id = get_id(face);
            double move_distance;
            if (get_face_displacement(entity, face, &displacement, &move_distance)) {
                KV_Pair *solid_copy = get_solid_by_id(entity_copy, solid_id);
                assert(solid);

                KV_Pair *copy_face = get_face_by_id(solid_copy, face_id);
                assert(copy_face);

                displace_plane(copy_face, &displacement);
                displace_verts(copy_face, &displacement);
                faces_fixed++;
                printf("32: brush %d <%s> solid %d face %d moved by %g\n", entity_id, classname, solid_id, face_id, move_distance);
            }
        }
    }

    KV_PairDestroy(move_solid_faces);
    KV_PairDestroy(move_faces);
}

void handle_entity(KV_Pair *changes, KV_Pair *entity, af_bool is_world) {
    int entity_id = get_id(entity);
    if (entity_id > entity_id_max) {
        entity_id_max = entity_id;
    }

    // check if this brush ent can be non-solid
    af_bool is_visual = AF_FALSE;
    const char *classname = "<unknown>";
    KV_Pair *classname_value = KV_FindPairOfType(entity, "classname", KV_TYPE_STRING);
    if (classname_value) {
        classname = KV_GetString(classname_value);
        if (!strcmp(classname, "func_detail_illusionary")) {
            is_visual = AF_TRUE;
        } else if (!strcmp(classname, "func_illusionary")) {
            is_visual = AF_TRUE;
        } else if (!strcmp(classname, "func_brush")) {
            const char *solidity = KV_FindString(entity, "Solidity", "");
            if (!strcmp(solidity, "1")) {
                is_visual = AF_TRUE;
            }
        } else if (!strcmp(classname, "info_map_parameters")) {
            const char *targetname = KV_FindString(entity, "targetname", "");
            if (!strcmp(targetname, "anglefixed")) {
                has_watermark = AF_TRUE;
            }
        }
    }

    KV_Pair *sub;
    KV_Pair *entity_copy = NULL;
    int i = 0;
    while ((sub = KV_GetPair(entity, i++))) {
        if (KV_GetDataType(sub) == KV_TYPE_NONE && !strcmp(KV_GetKey(sub), "solid")) {
            handle_solid(entity, classname, entity_id, changes, sub, is_visual, &entity_copy);
        }
    }

    if (entity_copy) {
        KV_AddTail(changes, entity_copy);
    }
}

// primary purpose is to inform people that decompile that the ramps have been anglefixed
// since bspsrc doesn't support func_detail_illusionary and makes them solid this can be confusing
// secondary purpose is the github link :)
KV_Pair *watermark() {
    KV_Pair *ent = KV_NewList("entity");
    KV_AddTail(ent, KV_NewString("classname", "info_map_parameters"));
    KV_AddTail(ent, KV_NewString("origin", "0 0 0"));
    KV_AddTail(ent, KV_NewString("angles", "0 0 0"));
    KV_AddTail(ent, KV_NewString("targetname", "anglefixed"));
    KV_AddTail(ent, KV_NewString("url", "https://github.com/fatalis/hammer-surf-anglefix"));
    return ent;
}

void anglefix_apply(KV_Pair *vmf, KV_Pair *collision_only_vmf) {
    KV_Pair *sub;
    int i = 0;

    KV_Pair *changes = KV_NewList(NULL); // internal

    // first pass: count id's and find changes
    while ((sub = KV_GetPair(vmf, i++))) {
        if (KV_GetDataType(sub) == KV_TYPE_NONE) {
            const char *key = KV_GetKey(sub);
            af_bool is_entity = !strcmp(key, "entity");
            af_bool is_world = !strcmp(key, "world");
            if (is_entity || is_world) {
                handle_entity(changes, sub, is_world);
            }
        }
    }

    // apply changes
    i = 0;
    KV_Pair *entity;
    while ((entity = KV_GetPair(changes, i++))) {
        entity_id_max++;
        set_id(entity, entity_id_max);

        KV_Pair *classname = KV_FindPairOfType(entity, "classname", KV_TYPE_STRING);
        if (classname) {
            KV_SetString(classname, "func_detail");
        } // TODO: fatal

        int k = 0;
        KV_Pair *solid;
        while ((solid = KV_GetPair(entity, k++))) {
            if (KV_GetDataType(solid) == KV_TYPE_NONE && !strcmp(KV_GetKey(solid), "solid")) {
                solid_id_max++;
                set_id(entity, entity_id_max);

                int j = 0;
                KV_Pair *face;
                while ((face = KV_GetPair(solid, j++))) {
                    if (KV_GetDataType(face) == KV_TYPE_NONE && !strcmp(KV_GetKey(face), "side")) {
                        side_id_max++;
                        set_id(face, side_id_max);

                        KV_Pair *material = KV_FindPairOfType(face, "material", KV_TYPE_STRING);
                        if (material) {
                            KV_SetString(material, debug_mode ? "DEV/DEV_MEASUREGENERIC01" : "TOOLS/TOOLSPLAYERCLIP");
                        }

                        remove_temp_flags(face);

                        // TODO: replace uaxis etc?
                    }
                }
            }
        }

        KV_AddTail(vmf, KV_PairCopy(entity));
        if (collision_only_vmf) {
            KV_AddTail(collision_only_vmf, KV_PairCopy(entity));
        }
    }

    KV_PairDestroy(changes);

    printf("faces angle fixed:        %d\n", faces_fixed);
    printf("solids angle fixed:       %d\n", solids_fixed);
    printf("solids faces angle fixed: %d\n", solid_faces_fixed);
}

static void reset() {
    faces_fixed = 0;
    solids_fixed = 0;
    solid_faces_fixed = 0;

    solid_id_max = 0;
    side_id_max = 0;
    entity_id_max = 0;

    has_watermark = AF_FALSE;
}

char *anglefix_generate_output(const char *path, char **output_collision_only) {
    KV_Pair *vmf = KV_ParseFile(path);

    if (!vmf) {
        fatal("%s\n", KV_GetError());
    }

    reset();

    KV_Pair *collision_only_vmf = KV_NewList(NULL); // internal
    anglefix_apply(vmf, output_collision_only ? collision_only_vmf : NULL);
    if (!has_watermark) {
        KV_AddTail(vmf, watermark());
    }

    char *output = KV_Print(vmf, NULL, 1024*1024, "\t"); /* 1MB step */
    if (!output) {
        KV_PairDestroy(vmf);
        fatal("%s\n", KV_GetError());
    }

    if (output_collision_only) {
        char *buf = KV_Print(collision_only_vmf, NULL, 1024*1024, "\t"); /* 1MB step */
        if (!buf) {
            KV_PairDestroy(collision_only_vmf);
            fatal("%s\n", KV_GetError());
        }
        *output_collision_only = buf;
    }

    KV_PairDestroy(collision_only_vmf);
    KV_PairDestroy(vmf);
    return output;
}
