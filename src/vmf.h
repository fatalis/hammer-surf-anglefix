#ifndef VMF_H
#define VMF_H

#include <keyvalues.h>

#include "anglefix.h"
#include "mathh.h"

int get_id(KV_Pair *pair);
af_bool set_id(KV_Pair *pair, int id);
af_bool dump_plane(char *buffer, size_t bufsize, const vec3x3_t *plane);
af_bool parse_plane(const char *plane, vec3x3_t *v);
af_bool parse_vert(const char *plane, vec3_t *v);
KV_Pair *get_face_by_id(KV_Pair *solid, int id);
KV_Pair *get_solid_by_id(KV_Pair *entity, int id);

#endif /* VMF_H */
