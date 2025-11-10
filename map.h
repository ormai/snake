// SPDX-License-Identifier: GPL-3.0-only
// Copyright © 2024  Mario D'Andrea https://ormai.me

#ifndef MAP_H
#define MAP_H

#include <sys/ioctl.h>

#include "snake.h"

struct map {
  int width;
  int height;
  /// Cells contained by the map contain. It is simply `width` * `height`.
  unsigned area;
  /// An offset from the top-left corner to center the map.
  struct point offset;
  /// Position of the apple on the map.
  struct point apple;
  /// The map is a 2D array. Each cell contains either `0`, for empty, or `1`
  /// for taken.
  int **grid;
};

/// Creates a new map. This function allocates memory.
[[nodiscard]] struct map *map_create(void);

/// Destroys a map created with `map_create`.
void map_destroy(struct map *map);

/// Checks whether the snake has hit any wall.
bool is_inside(const struct map *map, const struct snake *snake);

/// Spawns a new apple and draws it on the map.
void spawn_apple(struct map *map);

#endif // MAP_H
