#include <stdbool.h>
#include "math.h"
#include <math.h>

typedef enum { VERTICAL, HORIZONTAL } side_t;

typedef struct {
    Point2f collision_point;
    int     grid_index_collision;
    side_t  side;
    bool    valid;
} DDA_t;


// Move to map logic
bool check_map_bound_index(int index_x, int index_y, int index_max_x, int index_max_y) {
    if (index_x >= 0 && index_x < index_max_x && index_y >= 0 && index_y < index_max_y)
        return true;
    return false;
}

/*                                                                              Wall */
/* |─────────────────────────────────────────x─────────────────────────────────|     */
/*                          xx  P1        P2 x                                       */
/*        (P1)The calculated xx              x   To fix the fish eye, just           */
/* Ray has distortions based   xx            x     calculate perpendicular ray (P2)  */
/* with the center, due to      xx           x     P1 * cos(angle)                   */
/* ray "casting" the screen.     xx          x                                       */
/*                                xx         x                                       */
/*                                 xx        x                                       */
/*         The distortion cause     xx       x                                       */
/*         the Fish Eye illusion     xx      x                                       */
/*                                    xx     x                                       */
/*                                     xx    x                                       */
/*                                      xx   x                                       */
/*                                       xx  x                                       */
/*                                        xx x                                       */
/*                                          xx          "Camera view"                */
/*                                |──────────x──────────|                            */
float fix_eye_fish(Point2f ray, float angle) {
    return point2f_len(ray) * cosf(angle);
}

float normalize_rand(float rand) {
    if (rand > TAU)
        return rand - TAU;
    if (rand < 0)
        return rand + TAU;
    return rand;
}

DDA_t DDA(int map_tile[24][24], int map_height, int map_len, Point2f direction, Point2f start) {
    DDA_t dda_return;
    dda_return.side = 0;
    dda_return.grid_index_collision = -1;
    float ray_total_max = 800.0;
    float ray_total = 0.0;
    bool  ray_bound = false;

    Point2f delta = {direction.x / map_height - start.x / map_height,
                     direction.y / map_height - start.y / map_height};
    Point2f unitary_vector = point2f_normalize(delta);
    Point2f unitary_step_size = {sqrtf(1 + powf(unitary_vector.y / unitary_vector.x, 2)),
                                 sqrtf(1 + powf(unitary_vector.x / unitary_vector.y, 2))};
    Point2f ray_length_1D;
    Point2f step;
    Point2i tile_map_check = {(int)start.x, (int)start.y};
    Point2i absolute_map_tile;

    // set the initial ray direction y or x
    /* set walk to x */
    if (unitary_vector.x < 0) {
        step.x          = -1;
        ray_length_1D.x = (start.x - tile_map_check.x) * unitary_step_size.x;
    } else {
        step.x          = 1;
        ray_length_1D.x = ((tile_map_check.x + 1) - start.x) * unitary_step_size.x;
    }

    /* set walk to y */
    if (unitary_vector.y < 0) {
        step.y          = -1;
        ray_length_1D.y = (start.y - tile_map_check.y) * unitary_step_size.y;
    } else {
        step.y          = 1;
        ray_length_1D.y = ((tile_map_check.y + 1) - start.y) * unitary_step_size.y;
    }

    /* start walk until collision or max */
    while (!ray_bound && ray_total < ray_total_max) {
        if (ray_length_1D.x < ray_length_1D.y) {
            tile_map_check.x += step.x;
            ray_total = ray_length_1D.x;
            ray_length_1D.x += unitary_step_size.x;
            dda_return.side = VERTICAL; /* vertical */
        } else {
            tile_map_check.y += step.y;
            ray_total = ray_length_1D.y;
            ray_length_1D.y += unitary_step_size.y;
            dda_return.side = HORIZONTAL; /* horizontal */
        }

        /* check in the map if has a wall */
        absolute_map_tile.x = tile_map_check.x / map_height;
        absolute_map_tile.y = tile_map_check.y / map_height;
        if (check_map_bound_index(absolute_map_tile.x, absolute_map_tile.y, map_len, map_len)) {
            if (map_tile[absolute_map_tile.y][absolute_map_tile.x] != 0) {
                ray_bound                       = true;
                dda_return.grid_index_collision = map_tile[absolute_map_tile.y][absolute_map_tile.x];
            }
        }
    }

    if (ray_bound == true) {
        dda_return.collision_point.x = start.x + unitary_vector.x * ray_total;
        dda_return.collision_point.y = start.y + unitary_vector.y * ray_total;
        dda_return.valid             = true;
    } else {
        dda_return.collision_point.x = 0;
        dda_return.collision_point.y = 0;
        dda_return.valid             = false;
    }
    return dda_return;
}
