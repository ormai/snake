// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2024  Mario D'Andrea https://ormai.dev

#include <stdbool.h>
#include <stdlib.h>

#include "snake.h"

struct snake *snake_create(const struct point head, const size_t size) {
  struct snake *snake = calloc(1, sizeof(struct snake));
  snake->body = malloc(sizeof(struct point[size + 1]));
  snake->body[0] = head;
  snake->length = 1;
  snake->growing = false;
  return snake;
}

void snake_destroy(struct snake *snake) {
  if (snake != NULL) {
    free(snake->body);
    free(snake);
    snake = NULL;
  }
}

bool self_collision(const struct snake *snake) {
  const struct point head = snake->body[snake->length - 1];
  for (size_t i = 0; i < snake->length - 1; ++i) {
    if (snake->body[i].x == head.x && snake->body[i].y == head.y) {
      return true;
    }
  }
  return false;
}

void advance(struct snake *snake) {
  if (!snake->growing) {
    snake->old_tail = snake->body[0];
  }

  if (snake->growing) {
    snake->growing = false;
    snake->body[snake->length - 1] = snake->body[snake->length - 2];
  } else if (snake->length > 1) {
    for (size_t i = 0; i < snake->length - 1; ++i) {
      snake->body[i] = snake->body[i + 1];
    }
  }

  // Move it forward in the current direction
  switch (snake->direction) {
  case UP:
    --snake->body[snake->length - 1].y;
    break;
  case RIGHT:
    ++snake->body[snake->length - 1].x;
    break;
  case DOWN:
    ++snake->body[snake->length - 1].y;
    break;
  case LEFT:
    --snake->body[snake->length - 1].x;
    break;
  }
}

void change_direction(struct snake *self, enum direction direction) {
  // Disallow moving from DOWN to UP and from RIGHT to LEFT and vice versa when
  // the snake is longer than 1.
  if (direction != self->direction &&
      !(self->length > 1 && direction == (self->direction + 2) % (LEFT + 1))) {
    self->direction = direction;
  }
}
