// SPDX-License-Identifier: GPL-3.0-only
// Copyright © 2024  Mario D'Andrea https://ormai.dev

#ifndef MAP_H
#define MAP_H

#include <sys/ioctl.h>

#include "snake.h"

struct map {
  int width, height;
  unsigned area;
  struct point offset; // top-left offset to center the map
  struct point apple;
  int **grid;
};

struct map *map_create(void);
void map_destroy(struct map *map);

// Check for collision with the walls
bool is_inside(const struct map *map, const struct snake *snake);

// Spawn a new apple and draw it on the map
void spawn_apple(struct map *map);

#endif // MAP_H
