#ifndef ANGLEFIX_H
#define ANGLEFIX_H

#define MAX_PATH 260
#define SMOOTHING_GROUP_MOVE_FACES       (1 << (32 - 1))
#define SMOOTHING_GROUP_MOVE_SOLID_FACES (1 << (31 - 1))
#define SMOOTHING_GROUP_DEBUG            (1 << (30 - 1))
#define PLAYER_SIZE 32
/* #define AF_DEBUG */

// approximate
// "(%g %g %g) (%g %g %g) (%g %g %g)"
#define BUF_SIZE_PLANE 256
// "%g %g %g"
#define BUF_SIZE_VERT BUF_SIZE_PLANE / 3

typedef enum {
    AF_FALSE = 0,
    AF_TRUE  = 1
} af_bool;

#endif /* ANGLEFIX_H */
