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

#include <stdlib.h>

#include "snake.h"

Node *newNode(const Point spawnPosition, Node *prev) {
  Node *self = malloc(sizeof(Node));
  self->pos = spawnPosition;
  self->prev = prev;
  self->next = NULL;
  return self;
}

void destroyNode(Node *self) {
  if (self != NULL) {
    free(self);
    self = NULL;
  }
}

Snake *newSnake(const Point headPosition) {
  Snake *self = malloc(sizeof(Snake));
  self->tail = self->head = newNode(headPosition, NULL);
  self->direction = rand() % (WEST + 1);
  self->length = 1;
  return self;
}

void destroySnake(Snake *self) {
  if (self != NULL) {
    for (Node *it = self->head; it != NULL; it = it->prev)
      destroyNode(it->next);
    free(self);
    self = NULL;
  }
}

bool selfCollision(const Snake *self, Point *collision) {
  for (Node *it1 = self->head; it1 != NULL; it1 = it1->prev)
    for (Node *it2 = it1->prev; it2 != NULL; it2 = it2->prev)
      if (it1->pos.x == it2->pos.x && it1->pos.y == it2->pos.y) {
        if (collision != NULL)
          *collision = it1->pos;
        return true;
      }
  return false;
}

void pushFront(Snake *self, Point newHeadPosition) {
  self->head->next = newNode(newHeadPosition, self->head);
  self->head = self->head->next;
}

Node *popBack(Snake *self) {
  Node *oldTail = self->tail;
  self->tail = self->tail->next;
  self->tail->prev = NULL;
  return oldTail;
}

Node *advance(Snake *self) {
  Point newHeadPosition = self->head->pos; // Copy the current head position

  // Move it forward in the current direction
  switch (self->direction) {
  case NORTH:
    --newHeadPosition.y;
    break;
  case EAST:
    ++newHeadPosition.x;
    break;
  case SOUTH:
    ++newHeadPosition.y;
    break;
  case WEST:
    --newHeadPosition.x;
    break;
  }

  pushFront(self, newHeadPosition); // Push new head
  return popBack(self);             // Pop the tail
}

void changeDirection(Snake *self, Direction newDirection) {
  // Disallow moving from SOUTH to NORTH and vice versa and from EAST to WEST
  // and vice versa when the Snake is longer than 1
  if (newDirection != self->direction &&
      !(self->length > 1 && newDirection == (self->direction + 2) % (WEST + 1)))
    self->direction = newDirection;
}

void grow(Snake *self, Node *oldTail) {
  oldTail->next = self->tail;
  self->tail = oldTail;

  ++self->length;
}
