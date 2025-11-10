// SPDX-License-Identifier: GPL-3.0-only
// Copyright © 2024  Mario D'Andrea https://ormai.me

#define _POSIX_C_SOURCE 200809L

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include <time.h>

#include "snake.h"
#include "term.h"
#include "window.h"

/// Translates an x coordinate to display on the map. Two cells represent one
/// point: "██". Eg. x = 4 maps to the 9th actual terminal column.
static int translate(const int x) { return x + x + 1; }

void draw_point(const struct map *map, const struct point position) {
  print(position.y + map->offset.y, translate(position.x) + map->offset.x,
        "██");
}

void update_score(const struct map *map, const size_t score) {
  set_color(DEFAULT_COLOR);
  print(map->offset.y - 2, map->offset.x, "Score: %d", score);
}

void draw_walls(const struct map *map) {
  set_color(YELLOW);
  struct point up_left = {map->offset.x, map->offset.y - 1},
               down_right = {translate(map->width) + map->offset.x + 2,
                             map->height + map->offset.y + 1};
  for (int x = up_left.x; x <= down_right.x; ++x) {
    print(up_left.y, x, "▄");
    print(down_right.y, x, "▀");
  }
  for (int y = up_left.y + 1; y < down_right.y; ++y) {
    print(y, up_left.x, "█");
    print(y, down_right.x, "█");
  }
}

void redraw_snake(const struct map *map, struct snake *snake) {
  map->grid[snake->old_tail.y][snake->old_tail.x] = false;

  if (snake->length > 1) {
    set_color(GREEN);
    draw_point(map, snake->body[snake->length - 2]);
  }
  set_color(BRIGHT_GREEN);
  draw_point(map, snake->head);
  map->grid[snake->head.y][snake->head.x] = true;

  print(snake->old_tail.y + map->offset.y,
        translate(snake->old_tail.x) + map->offset.x, "  ");
}

static inline void update_doodle(struct snake *doodle,
                                 const struct point dialog_begin,
                                 const int dialog_height,
                                 const int dialog_width) {
  doodle->old_tail = doodle->body[0];
  memmove(doodle->body, doodle->body + 1,
          sizeof(struct point[doodle->length - 1]));

  // Head moves forward. The doodle moves in a loop.
  switch (doodle->direction) {
  case UP:
    if (doodle->head.y >= dialog_begin.y) {
      --doodle->head.y;
      break;
    }
    doodle->direction = LEFT;
    // fallthrough
  case LEFT:
    if (doodle->head.x > dialog_begin.x) {
      doodle->head.x -= 2;
      break;
    }
    doodle->direction = DOWN;
    // fallthrough
  case DOWN:
    if (doodle->head.y - 1 < dialog_begin.y + dialog_height) {
      ++doodle->head.y;
      break;
    }
    doodle->direction = RIGHT;
    // fallthrough
  case RIGHT:
    if (doodle->head.x < dialog_begin.x + dialog_width - 1) {
      doodle->head.x += 2;
      break;
    }
    doodle->direction = UP;
    --doodle->head.y;
  }
  doodle->body[doodle->length - 1] = doodle->head;
  set_color(BRIGHT_GREEN);
  print(doodle->head.y, doodle->head.x, "██");
  if (doodle->length >= 2) {
    set_color(GREEN);
    const struct point pre_head = doodle->body[doodle->length - 2];
    print(pre_head.y, pre_head.x, "██");
  }
  print(doodle->old_tail.y, doodle->old_tail.x, "  ");
  nanosleep(&(struct timespec){0, 33333333}, NULL);
}

static const char *diff[] = {"  incremental >", "   < easy >    ",
                             "  < medium >   ", "   < hard      "};

bool welcome_dialog(enum difficulty *difficulty) {
  static const char *welcome[] = {
      "",
      "                              _",
      "                             | |",
      "              ___ _ __   __ _| | _____   ___",
      "             / __| '_ \\ / _` | |/ / _ \\ / __|",
      "             \\__ \\ | | | (_| |   <  __/| (__",
      "             |___/_| |_|\\__,_|_|\\_\\___(_)___|",
      "",
      "",
      "          by Mario D'Andrea <https://ormai.me>",
      "",
      "              Difficulty %s",
      "",
      "                  Quit [q]      Play [⏎]",
      "",
      ""};
  static const int height = 16, width = 57;
  const struct winsize ws = get_term_size();
  const struct point begin = {ws.ws_col / 2 - width / 2 + 1,
                              ws.ws_row / 2 - height / 2 + 1};

  struct snake *doodle = snake_create((struct point){begin.x, begin.y + 2}, 7);
  doodle->direction = DOWN;
  set_color(GREEN);
  for (int i = 0; i < 7; ++i) { // Make it long 7
    doodle->head = doodle->body[doodle->length] =
        (struct point){begin.x, doodle->head.y + 1};
    ++doodle->length;
    print(doodle->head.y, doodle->head.x, "██");
  }

  set_color(DEFAULT_COLOR);
  for (int y = begin.y, i = 0; y < begin.y + height; ++y, ++i) {
    if (i == 11) {
      print(y, begin.x + 3, welcome[i], diff[*difficulty]);
    } else {
      print(y, begin.x, welcome[i]);
    }
  }

  while (true) {
    switch (getch()) {
    case '\n':
    case 'y': {
      snake_destroy(doodle);
      return false;
    }
    case '>':
    case ARROW_RIGHT:
      if (*difficulty != HARD) {
        ++*difficulty;
        set_color(DEFAULT_COLOR);
        print(begin.y + 11, begin.x + 3, welcome[11], diff[*difficulty]);
      }
      break;
    case '<':
    case ARROW_LEFT:
      if (*difficulty != INCREMENTAL) {
        --*difficulty;
        set_color(DEFAULT_COLOR);
        print(begin.y + 11, begin.x + 3, welcome[11], diff[*difficulty]);
      }
      break;
    case 'n':
    case 'q':
      snake_destroy(doodle);
      return true;
    }
    update_doodle(doodle, begin, height, width);
  }
}

static bool end_game_dialog(const struct map *map, enum difficulty *difficulty,
                            const size_t score, const char *banner[]) {
  static const int height = 16, width = 57;
  const struct point begin = {map->offset.x + map->width - width / 2 + 1,
                              map->offset.y + map->height / 2 - height / 2 + 1};

  set_color(DEFAULT_COLOR);
  for (int y = begin.y, i = 0; y < begin.y + height; ++y, ++i) {
    if (i == 9) { // Plug in the score
      print(y, begin.x, banner[i], score);
    } else if (i == 11) { // Plug in the difficulty
      print(y, begin.x, banner[i], diff[*difficulty]);
    } else {
      print(y, begin.x, banner[i]);
    }
  }

  nonblocking_input(false);
  while (true) {
    switch (getch()) {
    case '\n':
    case 'y': {
      return false;
    }
    case '>':
    case ARROW_RIGHT:
      if (*difficulty != HARD) {
        ++*difficulty;
        set_color(DEFAULT_COLOR);
        print(begin.y + 11, begin.x, banner[11], diff[*difficulty]);
      }
      break;
    case '<':
    case ARROW_LEFT:
      if (*difficulty != INCREMENTAL) {
        --*difficulty;
        set_color(DEFAULT_COLOR);
        print(begin.y + 11, begin.x, banner[11], diff[*difficulty]);
      }
      break;
    case 'n':
    case 'q':
      return true;
    }
  }
}

bool over_dialog(const struct map *map, enum difficulty *difficulty,
                 const size_t score) {
  static const char *over[] = {
      "┏━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┓",
      "┃   _____                        _____                  ┃",
      "┃  |  __ \\                      |  _  |                 ┃",
      "┃  | |  \\/ __ _ _ __ ___   ___  | | | |_   _____ _ __   ┃",
      "┃  | | __ / _` | '_ ` _ \\ / _ \\ | | | \\ \\ / / _ \\ '__|  ┃",
      "┃  | |_\\ \\ (_| | | | | | |  __/ \\ \\_/ /\\ V /  __/ |     ┃",
      "┃   \\____/\\__,_|_| |_| |_|\\___|  \\___/  \\_/ \\___|_|     ┃",
      "┃                                                       ┃",
      "┃                                                       ┃",
      "┃                   Your score was %-4d                 ┃",
      "┃                                                       ┃",
      "┃               Difficulty %s              ┃",
      "┃                                                       ┃",
      "┃              Quit [q]      Play again [⏎]             ┃",
      "┃                                                       ┃",
      "┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛"};
  return end_game_dialog(map, difficulty, score, over);
}

bool win_dialog(const struct map *map, enum difficulty *difficulty,
                const size_t score) {
  static const char *win[] = {
      "┏━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┓",
      "┃         __   __            _    _                     ┃",
      "┃         \\ \\ / /           | |  | |                    ┃",
      "┃          \\ V /___  _   _  | |  | | ___  _ __          ┃",
      "┃           \\ // _ \\| | | | | |/\\| |/ _ \\| '_ \\         ┃",
      "┃           | | (_) | |_| | \\  /\\  / (_) | | | |        ┃",
      "┃           \\_/\\___/ \\__,_|  \\/  \\/ \\___/|_| |_|        ┃",
      "┃                                                       ┃",
      "┃                                                       ┃",
      "┃                   Your score was %-4d                 ┃",
      "┃                                                       ┃",
      "┃               Difficulty: %s             ┃",
      "┃                                                       ┃",
      "┃              Quit [q]      Play again [⏎]             ┃",
      "┃                                                       ┃",
      "┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛"};
  return end_game_dialog(map, difficulty, score, win);
}
