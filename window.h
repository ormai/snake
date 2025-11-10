// SPDX-License-Identifier: GPL-3.0-only
// Copyright © 2024  Mario D'Andrea https://ormai.me

#ifndef WINDOW_H
#define WINDOW_H

#include <stdbool.h>
#include <sys/ioctl.h>

#include "map.h"
#include "snake.h"

/// Difficulty of a game.
enum difficulty {
  /// Start from `EASY` and increase the speed toward `HARD`
  /// based on the game progress.
  INCREMENTAL,
  /// Pretty slow, almost boring.
  EASY,
  /// Faster then `EASY`, but managable.
  MEDIUM,
  /// Good to test your reflexes.
  HARD
};

/// Draws a point consiting of "██" at position
void draw_point(const struct map *map, const struct point position);

/// Redraws the score line on the screen with the updated value.
void update_score(const struct map *map, const size_t score);

/// Draws the four walls delimiting the map.
void draw_walls(const struct map *map);

/// Draws the snake on the screen after it has advanced.
void redraw_snake(const struct map *map, struct snake *snake);

/// Shows the welcome dialog. Returns `true` if the user wants to quit.
bool welcome_dialog(enum difficulty *difficulty);

/// Shows the end game loose dialog. Returns `true` if the user wants to quit.
bool over_dialog(const struct map *map, enum difficulty *difficulty,
                 const size_t score);

/// Shows the end game win dialog. Returns true if the user wants to quit.
bool win_dialog(const struct map *map, enum difficulty *difficulty,
                const size_t score);

#endif // WINDOW_H
