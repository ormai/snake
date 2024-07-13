// snake.h -- Interface of the class Snake
//
// Copyright (C) 2024  Mario D'Andrea <https://ormai.dev>
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

#ifndef SNAKE_H
#define SNAKE_H

#include <stdbool.h>
#include <stddef.h>

typedef enum { NORTH, EAST, SOUTH, WEST } Direction;

// Coordinates from the top left corner of the Screen/playing field
typedef struct {
  int x, y;
} Vec2;

// The Snake is modelled by a doubly linked list
typedef struct {
  unsigned length; // Also the score
  Vec2 old_tail;   // Previous position of the tail
  bool growing;
  Direction direction;
  Vec2 *body;
} Snake;

Snake *snake_create(const Vec2 head_position, const size_t size);
void snake_destroy(Snake *self);

// Pop the tail and push it as the new head
void snake_ouroboros(Snake *self);

// Move the snake one cell forward in the current direction
void snake_advance(Snake *self);

void snake_change_direction(Snake *self, Direction direction);

// Check if the Snake touches itself
// If a self collision happens sets the parameter collision
bool snake_self_collision(const Snake *snake);

#endif // !SNAKE_H
