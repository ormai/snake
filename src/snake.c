#include "snake.h"
#include <stdlib.h>

// ██

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

void advance(Snake *self) {

  switch (self->direction) {
  case NORTH:
    --self->head->y;
    break;
  case EAST:
    ++self->head->x;
    break;
  case SOUTH:
    ++self->head->y;
    break;
  case WEST:
    --self->head->x;
    break;
  }

  // Point oldHead = (Point){self->head->x, self->head->y};
  self->oldHead = self->head;

  Point prev2;
  Node *iter = self->head->next;
  while (iter != NULL) {
    prev2.x = iter->x;
    prev2.y = iter->y;

    iter->x = self->oldHead->x;
    iter->y = self->oldHead->y;

    self->oldHead->x = prev2.x;
    self->oldHead->y = prev2.y;

    iter = iter->next;
  }
}

void changeDirection(Snake *self, Direction direction) {
  // The left hand side of the || is clear, while the right hand side checks
  // that the direction doesn't go from SOUTH to NORTH or vice versa and from
  // EAST to WEST and vice versa.
  // The enum type for the Direction enumerates as follow:
  //    { NORTH = 0, EAST = 1, SOUTH = 2, WEST = 3 }
  // There is a distance of 2 between directions that lie on the same axis.
  // WEST + 1 is the number of directions.
  if (direction == self->direction ||
      direction == (self->direction + 2) % WEST + 1)
    return;

  self->direction = direction;
}

void grow(Snake *self) {
  self->tail->next =
      newNode((Point){self->oldHead->x, self->oldHead->y}, self->tail);

  ++self->length;
}
