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

#include "snake.h"

typedef enum { INCREMENTAL, EASY, MEDIUM, HARD } Difficulty;

typedef struct screen {
  int width, height;
  int mapWidth, mapHeight; // The map occupies a fraction of the screen
  Point offset;            // An offset from the top-left for the map
  Point orb;               // The orb to capture
  int **grid;              // Keep track of the occupied cells.
} Screen;

Screen *newScreen(void);
void destroyScreen(Screen *self);

void initializeNcurses(void);

void drawWalls(const Screen *self);

void updateScore(const Screen *self, const unsigned score);

// Spawn a new orb and show it on the Screen
void spawnOrb(Screen *self);

// Check for collisions with the walls
bool insideBoundaries(const Screen *self, const Snake *snake);

// Draw the Snake on to the Screen
void draw(const Screen *self, const Snake *snake, bool growing,
          const Node *oldTail);

// Place a colored point ("██") on the map.
// Color is one of the eight terminal colors provided by ncurses.
void drawPoint(const Screen *self, const Point pos, const int color);

// Returns true if the player wants to quit.
// When gameOver is true the welcome screen dialog is shown, if so score and
// collision are irrelevant.
// Sets the difficulty as chosen by the player.
bool dialog(Screen *self, Difficulty *difficulty, const bool gameOver,
            const unsigned score, const Point collision);

#endif // !SCREEN_H
