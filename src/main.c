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
#include <time.h>

#include "screen.h"
#include "snake.h"

static void reset_game(Screen **screen, Snake **snake, Point *collision,
                       float *progress) {
  destroy_screen(*screen);
  *screen = new_screen();
  destroy_snake(*snake);
  *snake =
      new_snake((Point){(*screen)->map_width / 2, (*screen)->map_height / 2});
  *collision = (Point){-1, -1};
  *progress = 0.0;
}

int main(void) {
  const struct timespec delay[] = {
      {0, 83333333}, {0, 83333333}, {0, 50000000}, {0, 33333333}};
  Point collision = {-1, -1};
  float progress = 0;
  Difficulty difficulty = INCREMENTAL;
  bool wall_collision = false;

  initialize_ncurses();
  // Instantiate the objects
  Screen *screen = new_screen();
  Snake *snake =
      new_snake((Point){screen->map_width / 2, screen->map_height / 2});

  bool quit = dialog(screen, WELCOME, &difficulty, 0, (Point){0, 0});
  if (!quit)
    quit = prepare_game(screen, snake);

  // Main loop
  while (!quit) {
    switch (getch()) { // Get keyboard input
    case 'w':
    case 'k':
    case KEY_UP:
      change_direction(snake, NORTH);
      break;
    case 'l':
    case 'd':
    case KEY_RIGHT:
      change_direction(snake, EAST);
      break;
    case 'j':
    case 's':
    case KEY_DOWN:
      change_direction(snake, SOUTH);
      break;
    case 'h':
    case 'a':
    case KEY_LEFT:
      change_direction(snake, WEST);
      break;
    case 'q':
      quit = true;
    }

    advance(snake);

    if (snake->head->pos.x == screen->orb.x && // Check for eaten orb
        snake->head->pos.y == screen->orb.y) {
      snake->growing = true;
      ++snake->length;
      spawn_orb(screen);
      update_score(screen, snake->length);
      progress = (float)snake->length / screen->playing_surface;

      if (snake->length == screen->playing_surface) { // Check for win
        if (!(quit = dialog(screen, WIN, &difficulty, snake->length,
                            (Point){0, 0})))
          if (!(quit = dialog(screen, WELCOME, &difficulty, 0, (Point){0, 0})))
            reset_game(&screen, &snake, &collision, &progress);
        quit = prepare_game(screen, snake);
      }
    }

    if ((wall_collision = !inside_boundaries(screen, snake)))
      draw_point(screen,
                 snake->length > 1 ? snake->head->prev->pos : snake->old_tail,
                 COLOR_RED);
    else
      draw(screen, snake);

    if ((wall_collision || self_collision(snake, &collision)) &&
        !(quit = dialog(screen, OVER, &difficulty, snake->length, collision))) {
      reset_game(&screen, &snake, &collision, &progress);
      quit = prepare_game(screen, snake);
    }

    if (difficulty == INCREMENTAL)
      nanosleep(
          &(const struct timespec){0, delay[EASY].tv_nsec -
                                          delay[MEDIUM].tv_nsec * progress},
          NULL);
    else
      nanosleep(delay + difficulty, NULL); // &delay[difficulty]
  }

  destroy_snake(snake);
  destroy_screen(screen);
  endwin();
  return EXIT_SUCCESS;
}
