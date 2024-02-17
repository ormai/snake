// vim:filetype=c

#ifndef MAP_H
#define MAP_H

#include <stdbool.h>
#include <stdlib.h>

#include "snake.h"

typedef struct map {
  int screenWidth, screenHeight;
  int width, height; // The map occupies a fraction of the screen

  // An offset from the top-left corner to apply to all coordinates
  // in order to center the map
  Point offset;

  bool *grid; // Keep track of the occupied cells. Useful when generating orbs.
  // grid will represent the map which is a 2D surface, but the constructor
  // allocates a 1D array of size width * height.
  // To subscript such an array i * width + j is used.

  Point orb; // The orb to capture
} Map;

Map *newMap(void);
void destroyMap(Map *self);

void initializeNcurses(void);

// Spawn a new orb
void spawnOrb(Map *self);

// Check for collisions
bool borders(const Map *self, const Snake *snake);

void drawWalls(const Map *self);

// Draw the Snake on to the Map
void draw(const Map *self, const Snake *snake, bool growing,
          const Node *oldTail);

void updateScore(const Map *self, const unsigned score);

bool gameOver(Map *self, Snake *snake);

#endif // !MAP_H
