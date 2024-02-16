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

  Node *head, *tail, *oldHead;

  Direction direction; // Current direction
} Snake;

Node *newNode(const Point pos, Node *prev);
void destroyNode(Node *self);

Snake *newSnake(const Point mapCenter, Direction direction);
void destroySnake(Snake *self);

void move();

bool collided();

// Move the snake one cell forward in the current direction
void advance(Snake *self);

void changeDirection(Snake *self, Direction direction);

// Add a Node to the Snake
void grow(Snake *self);

#endif // !SNAKE_H

// vi:ft=c
