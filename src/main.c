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

#include <locale.h>
#include <ncurses.h>
#include <stdbool.h>
#include <stdlib.h>
#include <threads.h>

#include "screen.h"
#include "snake.h"

int main(void) {
  setlocale(LC_ALL, ""); // Use the locale of the environment
  initializeNcurses();

  const struct timespec delayMin = {0, 33333333L}, delayMedium = {0, 50000000L},
                        delayMax = {0, 83333333L},
                        delayDiff = {0, delayMax.tv_nsec - delayMin.tv_nsec};

  // Instantiate the objects
  Screen *screen = newScreen();
  Snake *snake = newSnake((Point){screen->width / 2, screen->height / 2});

  if (!welcome(screen))
    goto QUIT;

  drawWalls(screen);
  spawnOrb(screen);
  updateScore(screen, snake->length);

  Point collision = {-1, -1};
  float progress = 0.0;
  bool bounds = true;

  // GAME LOOP
  while ((bounds && !selfCollision(snake, &collision)) ||
         gameOver(screen, snake, &collision, &progress)) {

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
      goto QUIT;
    }

    bool growing = false;
    Node *oldTail = advance(snake);
    if (snake->head->pos.x == screen->orb.x &&
        snake->head->pos.y == screen->orb.y) {
      growing = true;
      grow(snake, oldTail); // reappend oldTail to the Snake
      screen->grid[oldTail->pos.y][oldTail->pos.x] = 1;

      spawnOrb(screen);
      progress = (float)snake->length / (screen->width * screen->height);

      updateScore(screen, snake->length);
    } else
      destroyNode(oldTail);

    bounds = insideBoundaries(screen, snake);
    if (bounds)
      draw(screen, snake, growing, oldTail);
    else
      drawPointWithColor(
          screen, snake->length > 1 ? snake->head->prev->pos : oldTail->pos,
          COLOR_RED);

    switch (screen->difficulty) {
    case INCREMENTAL: {
      const struct timespec delayIncrement = {
          0, delayMax.tv_nsec - (unsigned)(delayDiff.tv_nsec * progress)};
      thrd_sleep(&delayIncrement, NULL);
      break;
    }
    case EASY:
      thrd_sleep(&delayMax, NULL); // 12 fps
      break;
    case MEDIUM:
      thrd_sleep(&delayMedium, NULL); // 20 fps
      break;
    case HARD:
      thrd_sleep(&delayMin, NULL); // 30 fps
      break;
    }
  }

QUIT:
  destroySnake(snake);
  destroyScreen(screen);
  endwin();
  return EXIT_SUCCESS;
}
