#ifndef ANGLEFIXER_H
#define ANGLEFIXER_H

#include <keyvalues.h>

#include "mathh.h"

void displace_plane(KV_Pair *face, const vec3_t *displacement);
void displace_verts(KV_Pair *face, const vec3_t *displacement);
af_bool get_face_displacement(KV_Pair *entity, KV_Pair *face, vec3_t *out, double *distance);
void remove_temp_flags(KV_Pair *face);
void handle_solid(KV_Pair *entity, const char *classname, int entity_id, KV_Pair *changes, KV_Pair *solid, af_bool is_visual, KV_Pair **pentity_copy);
void handle_entity(KV_Pair *changes, KV_Pair *entity, af_bool is_world);
KV_Pair *watermark();
void anglefix_apply(KV_Pair *vmf, KV_Pair *collision_only_vmf);
char *anglefix_generate_output(const char *path, char **output_collision_only);

extern af_bool debug_mode;

#endif /* ANGLEFIXER_H */
