/* snake.h -- Interface of the class Snake
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

#ifndef SNAKE_H
#define SNAKE_H

#include <stdbool.h>

typedef enum { NORTH, EAST, SOUTH, WEST } Direction;

// Coordinates relative to the top right corner of the Screen
typedef struct point {
  int x, y;
} Point;

// The atomic piece that constitutes the Snake
typedef struct node {
  struct node *prev, *next;
  Point pos;
} Node;

typedef struct snake {
  unsigned length; // Also the score

  Node *head, *tail;

  Direction direction; // Current direction
} Snake;

Node *newNode(const Point pos, Node *prev);
void destroyNode(Node *self);

Snake *newSnake(const Point center);
void destroySnake(Snake *self);

// Move the snake one cell forward in the current direction
// Returns the old tail
Node *advance(Snake *self);

void changeDirection(Snake *self, Direction direction);

// Add a Node to the Snake
void grow(Snake *self, Node *oldTail);

// Check if the Snake touches itself
// Sets the parameter collision to the point where the collision happened
bool selfCollision(const Snake *self, Point *collision);

#endif // !SNAKE_H
