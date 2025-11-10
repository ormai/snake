// SPDX-License-Identifier: GPL-3.0-only
// Copyright © 2024  Mario D'Andrea https://ormai.me

#include <stdlib.h>
#include <string.h>

#include "snake.h"

struct snake *snake_create(const struct point head, const size_t size) {
  struct snake *snake = calloc(1, sizeof(struct snake));
  snake->body = malloc(sizeof(struct point[size + 1]));
  snake->body[0] = head;
  snake->head = head;
  snake->length = 1;
  snake->growing = false;
  snake->direction = DOWN;
  return snake;
}

void snake_destroy(struct snake *snake) {
  if (snake != nullptr) {
    if (snake->body != nullptr) {
      free(snake->body);
    }
    free(snake);
    snake = nullptr;
  }
}

bool self_collision(const struct snake *snake) {
  for (size_t i = 0; i < snake->length - 1; ++i) {
    if (snake->body[i].x == snake->head.x &&
        snake->body[i].y == snake->head.y) {
      return true;
    }
  }
  return false;
}

void advance(struct snake *snake) {
  snake->old_tail = snake->body[0];

  if (snake->growing) {
    snake->growing = false;
    snake->body[snake->length - 1] = snake->head;
  } else if (snake->length > 1) {
    memmove(snake->body, snake->body + 1,
            sizeof(struct point[snake->length - 1]));
  }

  switch (snake->direction) {
  case UP:
    --snake->head.y;
    break;
  case RIGHT:
    ++snake->head.x;
    break;
  case DOWN:
    ++snake->head.y;
    break;
  case LEFT:
    --snake->head.x;
    break;
  }
  snake->body[snake->length - 1] = snake->head;
}

void change_direction(struct snake *self, enum direction direction) {
  // Disallow moving from DOWN to UP and from RIGHT to LEFT and vice versa when
  // the snake is longer than 1.
  if (direction != self->direction &&
      !(self->length > 1 && direction == (self->direction + 2) % (LEFT + 1))) {
    self->direction = direction;
  }
}
