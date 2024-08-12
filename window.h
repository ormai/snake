// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2024  Mario D'Andrea https://ormai.dev

#ifndef WINDOW_H
#define WINDOW_H

#include <stdbool.h>
#include <sys/ioctl.h>

#include "map.h"
#include "snake.h"

enum difficulty { INCREMENTAL, EASY, MEDIUM, HARD };

void draw_point(const struct map *map, const struct point pos);

void update_score(const struct map *map, const size_t score);

void draw_walls(const struct map *map);

// Draw the snake on to the screen
void redraw_snake(const struct map *map, struct snake *snake);

// Returns true if the player wants to quit.
// score is irrelevant when kind is WELCOME, collision is relevant when kind is
// OVER.
// Sets the difficulty as chosen

bool welcome_dialog(enum difficulty *difficulty);

bool over_dialog(const struct map *map, enum difficulty *difficulty,
                 const size_t score);

bool win_dialog(const struct map *map, enum difficulty *difficulty,
                const size_t score);

#endif // WINDOW_H
