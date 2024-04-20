/* snake.c -- Implementation of the class Snake
 *
 * Copyright (C) 2024  Mario D'Andrea <https://ormai.dev>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details. */

#include <stdbool.h>
#include <stdlib.h>

#include "snake.h"

Node *node_create(const Point spawn_position, Node *prev) {
  Node *self = malloc(sizeof(Node));
  self->pos = spawn_position;
  self->prev = prev;
  self->next = NULL;
  return self;
}

void node_destroy(Node *self) {
  if (self != NULL) {
    free(self);
    self = NULL;
  }
}

Snake *snake_create(const Point head_position) {
  Snake *self = calloc(1, sizeof(Snake));
  self->tail = self->head = node_create(head_position, NULL);
  self->length = 1;
  self->growing = false;
  return self;
}

void snake_destroy(Snake *self) {
  if (self != NULL) {
    for (Node *it = self->head; it != NULL; it = it->prev)
      node_destroy(it->next);
    node_destroy(self->tail);
    free(self);
    self = NULL;
  }
}

bool snake_self_collision(const Snake *self, Point *collision) {
  for (Node *it1 = self->head; it1 != NULL; it1 = it1->prev)
    for (Node *it2 = it1->prev; it2 != NULL; it2 = it2->prev)
      if (it1->pos.x == it2->pos.x && it1->pos.y == it2->pos.y) {
        if (collision != NULL)
          *collision = it1->pos;
        return true;
      }
  return false;
}

void snake_ouroboros(Snake *self) {
  self->head->next = self->tail;
  self->tail = self->tail->next;
  self->tail->prev = NULL;

  *self->head->next =
      (Node){.next = NULL, .prev = self->head, .pos = self->head->pos};
  self->head = self->head->next;
}

void snake_advance(Snake *self) {
  if (!self->growing)
    self->old_tail = self->tail->pos;

  if (self->growing) {
    self->growing = false;
    self->head = self->head->next = node_create(self->head->pos, self->head);
  } else if (self->length > 1)
    snake_ouroboros(self);

  // Move it forward in the current direction
  switch (self->direction) {
  case NORTH:
    --self->head->pos.y;
    break;
  case EAST:
    ++self->head->pos.x;
    break;
  case SOUTH:
    ++self->head->pos.y;
    break;
  case WEST:
    --self->head->pos.x;
    break;
  }
}

void snake_change_direction(Snake *self, Direction direction) {
  /* Disallow moving from SOUTH to NORTH and vice versa and from EAST to WEST
   * and vice versa when the Snake is longer than 1. */
  if (direction != self->direction &&
      !(self->length > 1 && direction == (self->direction + 2) % (WEST + 1)))
    self->direction = direction;
}
