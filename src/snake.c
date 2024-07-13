// snake.c -- Implementation of the class Snake
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

#include <stdbool.h>
#include <stdlib.h>
#include <ncurses.h>

#include "snake.h"

Snake *snake_create(const Vec2 head_position, const size_t size) {
  Snake *self = calloc(1, sizeof(Snake));
  self->body = malloc(sizeof(Vec2[size + 1]));
  self->body[0] = head_position;
  self->length = 1;
  self->growing = false;
  return self;
}

void snake_destroy(Snake *self) {
  if (self != NULL) {
    free(self->body);
    free(self);
    self = NULL;
  }
}

bool snake_self_collision(const Snake *snake) {
  const Vec2 head = snake->body[snake->length - 1];
  for (size_t i = 0; i < snake->length - 1; ++i) {
    if (snake->body[i].x == head.x && snake->body[i].y == head.y) {
      return true;
    }
  }
  return false;
}

void snake_ouroboros(Snake *self) {
  for (size_t i = 0; i < self->length - 1; ++i) {
    self->body[i] = self->body[i + 1];
  }
}

void snake_advance(Snake *self) {
  if (!self->growing) {
    self->old_tail = self->body[0];
  }

  if (self->growing) {
    self->growing = false;
    self->body[self->length - 1] = self->body[self->length - 2];
  } else if (self->length > 1) {
    snake_ouroboros(self);
  }

  // Move it forward in the current direction
  switch (self->direction) {
  case NORTH:
    --self->body[self->length - 1].y;
    break;
  case EAST:
    ++self->body[self->length - 1].x;
    break;
  case SOUTH:
    ++self->body[self->length - 1].y;
    break;
  case WEST:
    --self->body[self->length - 1].x;
    break;
  }
}

void snake_change_direction(Snake *self, Direction direction) {
  // Disallow moving from SOUTH to NORTH and vice versa and from EAST to WEST
  // and vice versa when the Snake is longer than 1.
  if (direction != self->direction &&
      !(self->length > 1 && direction == (self->direction + 2) % (WEST + 1))) {
    self->direction = direction;
  }
}
