// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2024  Mario D'Andrea https://ormai.dev

#define _POSIX_C_SOURCE 200809L

#include <locale.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <threads.h>
#include <time.h>

#include "map.h"
#include "snake.h"
#include "term.h"
#include "window.h"

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

int main(void) {
  setlocale(LC_ALL, "");
  term_init();

  static const struct timespec delay[] = {
      {0, 83333333}, {0, 83333333}, {0, 50000000}, {0, 33333333}};

  struct map *map = NULL;
  struct snake *snake = NULL;
  struct game_state game = {0, false, false, false, true, INCREMENTAL};
  if (!(game.quit = welcome_dialog(&game.difficulty))) {
    new_game(&game, &map, &snake);
  }

  while (!game.quit) { // Main loop
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

    if (game.pre_game) {
      game.pre_game = false;
      erase_line(map->offset.y + map->height + 2); // Hide tooltip below the map
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
    if ((game.wall_collision = !inside_walls(map, snake))) {
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

    if (game.difficulty == INCREMENTAL) {
      const struct timespec sleep = {
          0, delay[EASY].tv_nsec - delay[HARD].tv_nsec * game.progress};
      nanosleep(&sleep, NULL);
    } else {
      nanosleep(&delay[game.difficulty], NULL);
    }
  }

  snake_destroy(snake);
  map_destroy(map);
  term_finalize();
  return 0;
}
