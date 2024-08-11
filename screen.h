// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2024  Mario D'Andrea https://ormai.dev

#ifndef SCREEN_H
#define SCREEN_H

#include <stdbool.h>
#include <sys/ioctl.h>

#include "snake.h"

enum difficulty { INCREMENTAL, EASY, MEDIUM, HARD };

struct screen {
  struct winsize ws;
  int map_width, map_height; // The map occupies a fraction of the screen
  unsigned playing_surface;  // One could call this the map
  struct point offset;       // An offset from the top-left for the map
  struct point orb;          // The orb to capture
  int **grid;                // To keep track of the occupied cells
};

struct screen *screen_create(void);
void screen_destroy(struct screen *screen);

void draw_point(const struct screen *screen, const struct point pos);

// Check for collision with the walls.
bool inside_walls(const struct screen *screen, const struct snake *snake);

// Spawn a new apple and show it on the screen;
void spawn_apple(struct screen *screen);

void update_score(const struct screen *screen, const size_t score);

void draw_walls(const struct screen *screen);

// Draw the snake on to the screen
void redraw_snake(const struct screen *screen, struct snake *snake);

bool prepare_game(struct screen *screen, struct snake *snake);

enum dialog_kind { WELCOME, OVER, WIN };

// Returns true if the player wants to quit.
// score is irrelevant when kind is WELCOME, collision is relevant when kind is
// OVER.
// Sets the difficulty as chosen 
bool screen_dialog(struct screen *screen, enum dialog_kind kind,
                   enum difficulty *difficulty, const unsigned score);

#endif // !SCREEN_H
