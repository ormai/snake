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

// Coordinates the top left corner of the Screen
typedef struct point {
  int x, y;
} Point;

// The atomic piece that constitutes a Snake
typedef struct node {
  struct node *prev, *next;
  Point pos;
} Node;

Node *newNode(const Point spawnPosition, Node *prev);
void destroyNode(Node *self);

// The Snake is modelled by a linked list
typedef struct snake {
  unsigned length; // Also the score
  Node *head, *tail;
  Point oldTail; // Previous position of the tail
  bool growing;
  Direction direction;
} Snake;

Snake *newSnake(const Point headPosition);
void destroySnake(Snake *self);

// Pop the tail and push it as the new head
void ouroboros(Snake *self);

// Move the snake one cell forward in the current direction
void advance(Snake *self);

void changeDirection(Snake *self, Direction newDirection);

// Check if the Snake touches itself
// If a self collision happens sets the parameter collision
bool selfCollision(const Snake *self, Point *collision);

#endif // !SNAKE_H
