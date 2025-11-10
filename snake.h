// SPDX-License-Identifier: GPL-3.0-only
// Copyright © 2024  Mario D'Andrea https://ormai.me

#ifndef SNAKE_H
#define SNAKE_H

#include <stdbool.h>
#include <stddef.h>

enum direction { UP, RIGHT, DOWN, LEFT };

/// Coordinate from the top left corner of the map.
struct point {
  int x, y;
};

struct snake {
  /// The lengh of the snake is also the score of the game.
  size_t length;
  /// Previous tail position.
  struct point old_tail;
  /// Whether the snake is growing in the current game tick.
  bool growing;
  /// Direction in which the head of the snake is pointing.
  enum direction direction;
  /// Head of the snake. Equivanent to `body[length - 1]`.
  struct point head;
  /// Body of the snake, consisting of an array of points.
  struct point *body;
};

/// Creates a new snake. This function allocats memory.
[[nodiscard]] struct snake *snake_create(const struct point head, const size_t size);

/// Destroys a snake created with `snake_create`.
void snake_destroy(struct snake *self);

/// Moves the snake one cell forward in the current direction.
void advance(struct snake *self);

/// Updates the direction of the snake.
void change_direction(struct snake *self, const enum direction direction);

/// Checks whether the snake's head overlaps with any other point of its body.
bool self_collision(const struct snake *self);

#endif // SNAKE_H
