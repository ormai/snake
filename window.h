// SPDX-License-Identifier: GPL-3.0-only
// Copyright © 2024  Mario D'Andrea https://ormai.me

#ifndef WINDOW_H
#define WINDOW_H

#include <stdbool.h>
#include <sys/ioctl.h>

#include "map.h"
#include "snake.h"

enum difficulty { INCREMENTAL, EASY, MEDIUM, HARD };

// Draw a point consiting of "██" at position
void draw_point(const struct map *map, const struct point position);

void update_score(const struct map *map, const size_t score);

void draw_walls(const struct map *map);

// Draw the snake on the screen after it has advanced
void redraw_snake(const struct map *map, struct snake *snake);

// Returns true if the user wants to quit, false otherwise.
bool welcome_dialog(enum difficulty *difficulty);

// Returns true if the user wants to quit, false otherwise.
bool over_dialog(const struct map *map, enum difficulty *difficulty,
                 const size_t score);

// Returns true if the user wants to quit, false otherwise.
bool win_dialog(const struct map *map, enum difficulty *difficulty,
                const size_t score);

#endif // WINDOW_H
