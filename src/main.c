/* main.c -- Game loop
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

#include <ncurses.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>

#include "screen.h"
#include "snake.h"

static void resetGame(Screen **screen, Snake **snake, Point *collision,
                      float *progress) {
  destroyScreen(*screen);
  *screen = newScreen();
  destroySnake(*snake);
  *snake = newSnake((Point){(*screen)->mapWidth / 2, (*screen)->mapHeight / 2});
  *collision = (Point){-1, -1};
  *progress = 0.0;
}

int main(void) {
  initializeNcurses();
  const useconds_t delayMin = 33333, delayMedium = 50000, delayMax = 83333,
                   delayDiff = delayMax - delayMin;
  Point collision = {-1, -1};
  float progress = 0.0;
  Difficulty difficulty = INCREMENTAL;
  bool wallCollision = false;

  // Instantiate the objects
  Screen *screen = newScreen();
  Snake *snake = newSnake((Point){screen->mapWidth / 2, screen->mapHeight / 2});

  bool quit = dialog(screen, WELCOME, &difficulty, 0, (Point){0, 0});
  if (!quit)
    quit = prepareGame(screen, snake);

  // Main loop
  while (!quit) {
    switch (getch()) { // Get keyboard input
    case 'w':
    case 'k':
    case KEY_UP:
      changeDirection(snake, NORTH);
      break;
    case 'l':
    case 'd':
    case KEY_RIGHT:
      changeDirection(snake, EAST);
      break;
    case 'j':
    case 's':
    case KEY_DOWN:
      changeDirection(snake, SOUTH);
      break;
    case 'h':
    case 'a':
    case KEY_LEFT:
      changeDirection(snake, WEST);
      break;
    case 'q':
      quit = true;
    }

    advance(snake);

    if (snake->head->pos.x == screen->orb.x && // Check for eaten orb
        snake->head->pos.y == screen->orb.y) {
      snake->growing = true;
      ++snake->length;
      spawnOrb(screen);
      updateScore(screen, snake->length);
      progress = (float)snake->length / screen->playingSurface;

      if (snake->length == screen->playingSurface) { // Check for win
        quit = dialog(screen, WIN, &difficulty, snake->length, (Point){0, 0});
        resetGame(&screen, &snake, &collision, &progress);
        if (!quit) {
          quit = dialog(screen, WELCOME, &difficulty, 0, (Point){0, 0});
          if (!quit)
            prepareGame(screen, snake);
        }
      }
    }

    wallCollision = !insideBoundaries(screen, snake);
    if (!wallCollision)
      draw(screen, snake);
    else // Highlight the collision in red
      drawPoint(screen,
                snake->length > 1 ? snake->head->prev->pos : snake->oldTail,
                COLOR_RED);

    if ((wallCollision || selfCollision(snake, &collision)) &&
        !(quit = dialog(screen, OVER, &difficulty, snake->length, collision))) {
      resetGame(&screen, &snake, &collision, &progress);
      prepareGame(screen, snake);
    }

    switch (difficulty) {
    case INCREMENTAL: {
      usleep(delayMax - (useconds_t)(delayDiff * progress));
      break;
    }
    case EASY:
      usleep(delayMax); // 12 fps
      break;
    case MEDIUM:
      usleep(delayMedium); // 20 fps
      break;
    case HARD:
      usleep(delayMin); // 30 fps
      break;
    }
  }

  destroySnake(snake);
  destroyScreen(screen);
  endwin();
  exit(0);
}
