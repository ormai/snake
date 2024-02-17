#ifndef SNAKE_H
#define SNAKE_H

#include <stdbool.h>

typedef enum { NORTH, EAST, SOUTH, WEST } Direction;

// Top-right coordinates
typedef struct point {
  int x, y;
} Point;

// The atomic piece that constitutes the Snake
typedef struct node {
  struct node *prev, *next;
  int x, y;
} Node;

typedef struct snake {
  unsigned length; // Also the score

  Node *head, *tail;

  Direction direction; // Current direction
} Snake;

Node *newNode(const Point pos, Node *prev);
void destroyNode(Node *self);

Snake *newSnake(const Point mapCenter);
void destroySnake(Snake *self);

// Move the snake one cell forward in the current direction
// Returns the old tail
Node *advance(Snake *self);

void changeDirection(Snake *self, Direction direction);

// Add a Node to the Snake
void grow(Snake *self, Node *oldTail);

// Check if the Snake touches itself
bool selfCollision(const Snake *self);

#endif // !SNAKE_H

// vi:ft=c
