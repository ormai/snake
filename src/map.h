#ifndef MAP_H
#define MAP_H

#include <stdbool.h>
#include <stdlib.h>

#include "snake.h"

typedef struct map {
  unsigned width, height;
  bool *grid;

  Point orb; // The orb to capture
} Map;

Map *newMap(const unsigned width, const unsigned height);
void destroyMap(Map *self);

// Spawn a new orb
void newOrb(Map *self);

// Check for collisions
bool collision(const Map *self, const Snake *snake);

void draw(const Map *self, const Snake *snake);

#endif // !MAP_H
