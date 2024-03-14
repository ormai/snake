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
  int width, height;         // Of the terminal window, in cells
  int map_width, map_height; // The map occupies a fraction of the screen
  unsigned playing_surface;  // One could call this the map
  Point offset;              // An offset from the top-left for the map
  Point orb;                 // The orb to capture
  int **grid;                // To keep track of the occupied cells
} Screen;

Screen *new_screen(void);
void destroy_screen(Screen *self);

void initialize_ncurses(void);

void draw_walls(const Screen *self);

void update_score(const Screen *self, const unsigned score);

// Spawn a new orb and show it on the Screen
void spawn_orb(Screen *self);

// Check for collisions with the walls
bool inside_boundaries(const Screen *self, const Snake *snake);

// Draw the Snake on to the Screen
void draw(const Screen *self, Snake *snake);

bool prepare_game(Screen *self, Snake *snake);

// Place a colored point ("██") on the map.
void draw_point(const Screen *self, const Point pos, const int color);

typedef enum { WELCOME, OVER, WIN } DialogKind;

// Returns true if the player wants to quit.
// score is irrelevant when kind is WELCOME, collision is relevant when kind is
// OVER.
// Sets the difficulty as chosen by the player when kind is not WIN.
bool dialog(Screen *self, DialogKind kind, Difficulty *difficulty,
            const unsigned score, const Point collision);

#endif // !SCREEN_H
