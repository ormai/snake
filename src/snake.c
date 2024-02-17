#include <stdio.h>
#include <stdlib.h>

#include "snake.h"

Node *newNode(const Point pos, Node *prev) {
  Node *self = malloc(sizeof(Node));

  self->x = pos.x;
  self->y = pos.y;
  self->prev = prev;
  self->next = NULL;

  return self;
}

void destroyNode(Node *self) {
  if (self != NULL)
    free(self);
}

Snake *newSnake(const Point mapCenter, Direction direction) {
  Snake *self = malloc(sizeof(Snake));
  self->tail = self->head = newNode(mapCenter, NULL);
  self->direction = direction;
  self->length = 1;
  return self;
}

void destroySnake(Snake *self) {
  if (self != NULL) {
    Node *current = self->head, *next;
    while (current != NULL) {
      next = current->next;
      destroyNode(current);
      current = next;
    }
    free(self);
  }
}

Node *advance(Snake *self) {
  // Copy the position of the current head
  Point newHeadPosition = {self->head->x, self->head->y};

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

  // Push new head
  self->head->next = newNode(newHeadPosition, self->head);
  self->head = self->head->next;

  // Pop the tail
  Node *oldTail = self->tail;
  self->tail = self->tail->next;
  self->tail->prev = NULL;

  return oldTail;
}

void changeDirection(Snake *self, Direction direction) {
  // The left hand side of the || is clear, while the right hand side checks
  // that the direction doesn't go from SOUTH to NORTH or vice versa and from
  // EAST to WEST and vice versa.
  // The enum type for the Direction enumerates as follow:
  //    { NORTH = 0, EAST = 1, SOUTH = 2, WEST = 3 }
  // There is a distance of 2 between directions that lie on the same axis.
  // WEST + 1 is the number of directions.

  // if (direction == self->direction ||
  //     direction == (self->direction + 2) % WEST + 1)
  //   return;

  self->direction = direction;
}

void grow(Snake *self, Node *oldTail) {
  oldTail->next = self->tail;
  self->tail = oldTail;

  ++self->length;
}
