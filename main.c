// SPDX-License-Identifier: GPL-3.0-only
// Copyright © 2024  Mario D'Andrea https://ormai.me

#define _POSIX_C_SOURCE 200809L

#include <locale.h>
#include <stdbool.h>
#include <threads.h>
#include <time.h>

#include "map.h"
#include "snake.h"
#include "term.h"
#include "window.h"

#define SECOND 1000000000LL // a second in nanoseconds

struct game_state {
  float progress;
  bool wall_collision, self_collision;
  bool quit;
  bool pre_game; // The game waits for the first input of the player
  enum difficulty difficulty;
};

static void new_game(struct game_state *game, struct map **m,
                     struct snake **s) {
  struct map *map = *m;
  map_destroy(map);
  *m = map = map_create();

  struct snake *snake = *s;
  snake_destroy(snake);
  const struct point map_center = {map->width / 2, map->height / 2};
  *s = snake = snake_create(map_center, map->area);

  erase();
  draw_walls(map);
  spawn_apple(map);
  update_score(map, snake->length);
  set_color(BRIGHT_GREEN);
  draw_point(map, snake->head);
  set_color(DEFAULT_COLOR);
  print(map->offset.y + map->height + 2, map->offset.x,
        "Move in any direction to start the game.");
  nonblocking_input(false);
  game->pre_game = true;
  game->progress = 0;
}

/// Return the current time in nanoseconds.
static long long time_ns(void) {
  struct timespec ts;
  clock_gettime(CLOCK_MONOTONIC, &ts);
  return ts.tv_sec * SECOND + ts.tv_nsec;
}

int main(void) {
  setlocale(LC_ALL, "");
  term_init();

  struct map *map = NULL;
  struct snake *snake = NULL;
  struct game_state game = {0, false, false, false, true, INCREMENTAL};
  if (!(game.quit = welcome_dialog(&game.difficulty))) {
    new_game(&game, &map, &snake);
  }

  // Update the program at two different speeds, the graphic is drawn 60 times
  // a second, while the game updates at a different rate based on difficulty.
  static const long long display_interval = SECOND / 60;
  static const long long logic_update_interval[] = {SECOND / 12, SECOND / 12,
                                                    SECOND / 20, SECOND / 30};
  long long last_logic_time = 0, current_time;

  while (!game.quit) { // Main loop
    current_time = time_ns();

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
      game.quit = true;
    }

    const long long logic_interval =
        game.difficulty == INCREMENTAL
            ? logic_update_interval[EASY] -
                  logic_update_interval[HARD] * game.progress
            : logic_update_interval[game.difficulty];

    if (current_time - last_logic_time >= logic_interval) {
      if (game.pre_game) {
        game.pre_game = false;
        erase_line(map->offset.y + map->height + 2); // Hide tooltip below map
        nonblocking_input(true);
      }

      if (snake->head.x == map->apple.x && snake->head.y == map->apple.y) {
        snake->growing = true;
        ++snake->length;
        spawn_apple(map);
        update_score(map, snake->length);
        game.progress = (snake->length + .0) / map->area;
        if (snake->length == map->area) {
          if (!(game.quit = win_dialog(map, &game.difficulty, snake->length))) {
            new_game(&game, &map, &snake);
          }
        }
      }

      advance(snake);

      // Game over after collision
      if ((game.wall_collision = !is_inside(map, snake))) {
        set_color(RED);
        draw_point(map, snake->length > 1 ? snake->body[snake->length - 2]
                                          : snake->old_tail);
      } else {
        redraw_snake(map, snake);
      }
      if ((game.self_collision = self_collision(snake))) {
        set_color(RED);
        draw_point(map, snake->head);
      }
      if ((game.wall_collision || game.self_collision) &&
          !(game.quit = over_dialog(map, &game.difficulty, snake->length))) {
        new_game(&game, &map, &snake);
      }
      last_logic_time = current_time;
    }

    const long long time_left = display_interval - time_ns() - current_time;
    if (time_left > 0) {
      nanosleep(&(struct timespec){time_left / SECOND, time_left % SECOND},
                NULL);
    }
  }

  snake_destroy(snake);
  map_destroy(map);
  term_finalize();
  return 0;
}
