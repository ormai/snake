// main.c -- Game loop
//
// Copyright (C) 2024  Mario D'Andrea <https://ormai.dev>
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

#include <ncurses.h>
#include <stdbool.h>
#include <stdlib.h>
#include <threads.h>

#include "screen.h"
#include "snake.h"

static void reset_game(Screen **screen, Snake **snake, float *progress) {
  screen_destroy(*screen);
  *screen = screen_create();
  snake_destroy(*snake);
  *snake =
      snake_create((Vec2){(*screen)->map_width / 2, (*screen)->map_height / 2},
                   (*screen)->map_width * (*screen)->map_height);
  *progress = 0.0;
}

int main(void) {
  const struct timespec delay[] = {
      {0, 83333333}, {0, 83333333}, {0, 50000000}, {0, 33333333}};
  float progress = 0;
  Difficulty difficulty = INCREMENTAL;
  bool wall_collision = false;

  screen_prepare();
  // Instantiate the objects
  Screen *screen = screen_create();
  Snake *snake =
      snake_create((Vec2){screen->map_width / 2, screen->map_height / 2},
                   screen->map_width * screen->map_height);

  bool quit = screen_dialog(screen, WELCOME, &difficulty, 0);
  if (!quit) {
    quit = screen_prepare_game(screen, snake);
  }

  // Main loop
  while (!quit) {
    switch (getch()) { // Get keyboard input
    case 'w':
    case 'k':
    case KEY_UP:
      snake_change_direction(snake, NORTH);
      break;
    case 'l':
    case 'd':
    case KEY_RIGHT:
      snake_change_direction(snake, EAST);
      break;
    case 'j':
    case 's':
    case KEY_DOWN:
      snake_change_direction(snake, SOUTH);
      break;
    case 'h':
    case 'a':
    case KEY_LEFT:
      snake_change_direction(snake, WEST);
      break;
    case 'q':
      quit = true;
    }

    if (snake->body[snake->length - 1].x ==
            screen->orb.x && // Check for eaten orb
        snake->body[snake->length - 1].y == screen->orb.y) {
      snake->growing = true;
      ++snake->length;
      screen_spawn_orb(screen);
      screen_update_score(screen, snake->length);
      progress = (float)snake->length / screen->playing_surface;

      if (snake->length == screen->playing_surface) { // Check for win
        if (!(quit = screen_dialog(screen, WIN, &difficulty, snake->length))) {
          if (!(quit = screen_dialog(screen, WELCOME, &difficulty, 0))) {
            reset_game(&screen, &snake, &progress);
          }
        }
        quit = screen_prepare_game(screen, snake);
      }
    }

    snake_advance(snake);

    if ((wall_collision = !screen_inside_boundaries(screen, snake))) {
      screen_draw_point(screen,
                        snake->length > 1 ? snake->body[snake->length - 2]
                                          : snake->old_tail,
                        COLOR_RED);
    } else {
      screen_draw(screen, snake);
    }

    const bool self_collision = snake_self_collision(snake);
    if (self_collision) {
      screen_draw_point(screen, snake->body[snake->length - 1], COLOR_RED);
    }
    if ((wall_collision || self_collision) &&
        !(quit = screen_dialog(screen, OVER, &difficulty, snake->length))) {
      reset_game(&screen, &snake, &progress);
      quit = screen_prepare_game(screen, snake);
    }

    if (difficulty == INCREMENTAL) {
      const struct timespec sleep = {0, delay[EASY].tv_nsec -
                                            delay[MEDIUM].tv_nsec * progress};
      thrd_sleep(&sleep, NULL);
    } else {
      thrd_sleep(&delay[difficulty], NULL); // &delay[difficulty]
    }
  }

  snake_destroy(snake);
  screen_destroy(screen);
  endwin();
  return EXIT_SUCCESS;
}
