/* screen.h -- Interface of the class Screen
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

#ifndef SCREEN_H
#define SCREEN_H

#include <stdbool.h>
#include <stdlib.h>

#include "snake.h"

typedef enum { INCREMENTAL, EASY, MEDIUM, HARD } Difficulty;

typedef struct screen {
  int screenWidth, screenHeight;
  int width, height; // The playing field occupies a fraction of the screen

  // An offset from the top-left corner to apply to all coordinates
  // in order to center the playing field
  Point offset;

  Point orb; // The orb to capture

  Difficulty difficulty;

  int **grid; // Keep track of the occupied cells. Useful when generating orbs.
} Screen;

Screen *newScreen(void);
void destroyScreen(Screen *self);

void initializeNcurses(void);

void drawWalls(const Screen *self);

void updateScore(const Screen *self, const unsigned score);

// Welcome screen
// Returns true if the player choose to play, false if they choose to quit
bool welcome(Screen *self);

// Spawn a new orb
void spawnOrb(Screen *self);

// Check for collisions
bool insideBoundaries(const Screen *self, const Snake *snake);

// Draw the Snake on to the Screen
void draw(const Screen *self, const Snake *snake, bool growing,
          const Node *oldTail);

bool gameOver(Screen *self, Snake *snake, Point *collision, float *progress);

void drawPointWithColor(const Screen *self, const Point pos, const int color);

#endif // !SCREEN_H
