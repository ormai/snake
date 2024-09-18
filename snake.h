// SPDX-License-Identifier: GPL-3.0-only
// Copyright © 2024  Mario D'Andrea https://ormai.dev

#ifndef SNAKE_H
#define SNAKE_H

#include <stdbool.h>
#include <stddef.h>

enum direction { UP, RIGHT, DOWN, LEFT };

// Coordinates from the top left corner of the map
struct point {
  int x, y;
};

struct snake {
  size_t length;         // Also the score
  struct point old_tail; // Previous position of the tail
  bool growing;
  enum direction direction;
  struct point head; // would be body[length - 1], for easy access
  struct point *body;
};

struct snake *snake_create(const struct point head, const size_t size);
void snake_destroy(struct snake *self);

// Move the snake one cell forward in the current direction
void advance(struct snake *self);

void change_direction(struct snake *self, const enum direction direction);

// Check whether the snake's head overlaps with any other point of its body
bool self_collision(const struct snake *snake);

#endif // SNAKE_H
