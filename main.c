// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2024  Mario D'Andrea https://ormai.dev

#define _POSIX_C_SOURCE 200809L

#include <locale.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <threads.h>
#include <time.h>

#include "screen.h"
#include "snake.h"
#include "term.h"

static void reset_game(struct screen **screen, struct snake **snake,
                       float *progress) {
  screen_destroy(*screen);
  *screen = screen_create();
  snake_destroy(*snake);
  *snake = snake_create(
      (struct point){(*screen)->map_width / 2, (*screen)->map_height / 2},
      (*screen)->map_width * (*screen)->map_height);
  *progress = 0.0;
}

int main(void) {
  setlocale(LC_ALL, "");
  term_init();

  const struct timespec delay[] = {
      {0, 83333333}, {0, 83333333}, {0, 50000000}, {0, 33333333}};
  float progress = 0;
  enum difficulty difficulty = INCREMENTAL;
  bool wall_collision = false;

  struct screen *screen = screen_create();
  struct snake *snake = snake_create(
      (struct point){screen->map_width / 2, screen->map_height / 2},
      screen->map_width * screen->map_height);

  bool quit = screen_dialog(screen, WELCOME, &difficulty, 0);
  if (!quit) {
    quit = prepare_game(screen, snake);
  }

  while (!quit) { // Main loop
    switch (getch()) {
    case 'w':
    case 'k':
    case ARROW_UP:
      change_direction(snake, UP);
      break;
    case 'l':
    case 'd':
    case ARROW_RIGHT:
      change_direction(snake, RIGHT);
      break;
    case 'j':
    case 's':
    case ARROW_DOWN:
      change_direction(snake, DOWN);
      break;
    case 'h':
    case 'a':
    case ARROW_LEFT:
      change_direction(snake, LEFT);
      break;
    case 'q':
      quit = true;
    }

    if (snake->body[snake->length - 1].x ==
            screen->orb.x && // Check for eaten orb
        snake->body[snake->length - 1].y == screen->orb.y) {
      snake->growing = true;
      ++snake->length;
      spawn_apple(screen);
      update_score(screen, snake->length);
      progress = (float)snake->length / screen->playing_surface;

      if (snake->length == screen->playing_surface) { // Check for win
        if (!(quit = screen_dialog(screen, WIN, &difficulty, snake->length))) {
          if (!(quit = screen_dialog(screen, WELCOME, &difficulty, 0))) {
            reset_game(&screen, &snake, &progress);
          }
        }
        quit = prepare_game(screen, snake);
      }
    }

    advance(snake);

    if ((wall_collision = !inside_walls(screen, snake))) {
      set_color(RED);
      draw_point(screen, snake->length > 1 ? snake->body[snake->length - 2]
                                           : snake->old_tail);
    } else {
      redraw_snake(screen, snake);
    }

    const bool sf = self_collision(snake);
    if (sf) {
      set_color(RED);
      draw_point(screen, snake->body[snake->length - 1]);
    }
    if ((wall_collision || sf) &&
        !(quit = screen_dialog(screen, OVER, &difficulty, snake->length))) {
      reset_game(&screen, &snake, &progress);
      quit = prepare_game(screen, snake);
    }

    if (difficulty == INCREMENTAL) {
      const struct timespec sleep = {0, delay[EASY].tv_nsec -
                                            delay[MEDIUM].tv_nsec * progress};
      nanosleep(&sleep, NULL);
    } else {
      nanosleep(&delay[difficulty], NULL);
    }
  }

  snake_destroy(snake);
  screen_destroy(screen);
  term_finalize();
  return 0;
}
