#ifndef SCREEN_H
#define SCREEN_H

#include <stdbool.h>
#include <stdlib.h>

#include "snake.h"

typedef struct screen {
  int screenWidth, screenHeight;
  int width, height; // The playing field occupies a fraction of the screen

  // An offset from the top-left corner to apply to all coordinates
  // in order to center the playing field
  Point offset;

  bool *grid; // Keep track of the occupied cells. Useful when generating orbs.
  // grid will represent the playing field which is a 2D surface, but the
  // constructor allocates a 1D array of size width * height.
  // To subscript such an array i * width + j is used.

  Point orb; // The orb to capture
} Screen;

Screen *newScreen(void);
void destroyScreen(Screen *self);

void initializeNcurses(void);

// Spawn a new orb
void spawnOrb(Screen *self);

// void drawPointWithColor(const Point pos, const int color);

// Check for collisions
bool borders(const Screen *self, const Snake *snake);

void drawWalls(const Screen *self);

// Draw the Snake on to the Screen
void draw(const Screen *self, const Snake *snake, bool growing,
          const Node *oldTail);

void updateScore(const Screen *self, const unsigned score);

bool gameOver(Screen *self, Snake *snake, const Point collision);

#endif // !SCREEN_H
