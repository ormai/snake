// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2024  Mario D'Andrea https://ormai.dev

#define _POSIX_C_SOURCE 200809L

#include <stdbool.h>
#include <stdlib.h>
#include <time.h>

#include "screen.h"
#include "snake.h"
#include "term.h"

static const char *diff[] = {"  incremental >", "   < easy >    ",
                             "  < medium >   ", "   < hard      "};
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
    "          by Mario D'Andrea <https://ormai.dev>",
    "",
    "              Difficulty %s",
    "",
    "                  Quit [q]      Play [⏎]",
    "",
    ""};
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
    "┃             Quit [q]      Return home [⏎]             ┃",
    "┃                                                       ┃",
    "┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛"};

struct screen *screen_create(void) {
  struct screen *screen = malloc(sizeof(struct screen));

  screen->ws = get_term_size();
  screen->map_width = screen->ws.ws_col / 4; // Further down is explained why 4
  screen->map_height = screen->ws.ws_row * 2 / 3;
  screen->playing_surface = screen->map_width * screen->map_height;

  screen->offset =
      (struct point){(screen->ws.ws_col - screen->map_width * 2) / 2,
                     (screen->ws.ws_row - screen->map_height) / 2};

  screen->grid = malloc(sizeof(int * [screen->map_height + 1]));

  for (int i = 0; i <= screen->map_height; ++i) {
    screen->grid[i] = calloc(screen->map_width + 1, sizeof(int));
  }

  return screen;
}

void screen_destroy(struct screen *screen) {
  if (screen != NULL) {
    if (screen->grid != NULL) {
      for (int i = 0; i <= screen->map_height; ++i) {
        free(screen->grid[i]);
      }
      free(screen->grid);
    }
    free(screen);
    screen = NULL;
  }
}

// Translate an x coordinate to display on the screen. Two cells represent one
// point: "██". Eg. x = 4 maps to the 9th actual terminal column.
static int translate(const int x) { return x + x + 1; }

void draw_point(const struct screen *screen, const struct point pos) {
  print(pos.y + screen->offset.y, translate(pos.x) + screen->offset.x, "██");
}

bool inside_walls(const struct screen *self, const struct snake *snake) {
  const struct point head = snake->body[snake->length - 1];
  return head.x <= self->map_width && head.x >= 0 &&
         head.y <= self->map_height && head.y >= 0;
}

void spawn_apple(struct screen *screen) {
  // This is a critical point. With a big enough map and when the struct snake
  // is short there is no problem. But when progressing towards the completion
  // of the game the app will probably stall, trying to randomly get a correct
  // position for the orb. One solution I thought is creating a dynamic
  // structure that holds the set of current available struct points to choose
  // from to spawn a new orb. But this is a lot of code and could slow things
  // down anyway. So I will leave the problem open for now.
  do {
    screen->orb.x = rand() % (screen->map_width + 1);
    screen->orb.y = rand() % (screen->map_height + 1);
  } while (screen->grid[screen->orb.y][screen->orb.x] == 1);

  set_color(MAGENTA);
  draw_point(screen, screen->orb);
}

void update_score(const struct screen *screen, const size_t score) {
  set_color(DEFAULT_COLOR);
  print(screen->offset.y - 2, screen->offset.x, "Score: %d", score);
}

void draw_walls(const struct screen *screen) {
  erase();
  set_color(YELLOW);
  struct point up_left = {screen->offset.x, screen->offset.y - 1},
               down_right = {translate(screen->map_width) + screen->offset.x +
                                 2,
                             screen->map_height + screen->offset.y + 1};
  for (int x = up_left.x; x <= down_right.x; ++x) {
    print(up_left.y, x, "▄");
    print(down_right.y, x, "▀");
  }
  for (int y = up_left.y + 1; y < down_right.y; ++y) {
    print(y, up_left.x, "█");
    print(y, down_right.x, "█");
  }
}

void redraw_snake(const struct screen *screen, struct snake *snake) {
  // Cover the old tail with a blank if the snake has not grown;
  if (!snake->growing) {
    print(snake->old_tail.y + screen->offset.y,
          translate(snake->old_tail.x) + screen->offset.x, "  ");
    // screen->grid[snake->old_tail.y][snake->old_tail.x] = 0; // mark it free
  }

  set_color(BRIGHT_GREEN);
  draw_point(screen, snake->body[snake->length - 1]);
  if (snake->length > 1) {
    set_color(GREEN);
    draw_point(screen, snake->body[snake->length - 2]);
  }
  // screen->grid[snake->body[snake->length - 1].y]
  //           [snake->body[snake->length - 1].x] = 1; // mark it occupied
}

bool prepare_game(struct screen *screen, struct snake *snake) {
  draw_walls(screen);
  spawn_apple(screen);
  update_score(screen, snake->length);
  set_color(BRIGHT_GREEN);
  draw_point(screen, snake->body[snake->length - 1]); // draw head
  set_color(DEFAULT_COLOR);
  print(screen->offset.y + screen->map_height + 2, screen->offset.x,
        "Move in any direction to start the game.");
  nonblocking_input(false);

get_user_input: // Get the initial direction of the snake
  switch (getch()) {
  case 'w':
  case 'k':
  case ARROW_UP:
    snake->direction = UP;
    break;
  case 'l':
  case 'd':
  case ARROW_RIGHT:
    snake->direction = RIGHT;
    break;
  case 'j':
  case 's':
  case ARROW_DOWN:
    snake->direction = DOWN;
    break;
  case 'h':
  case 'a':
  case ARROW_LEFT:
    snake->direction = LEFT;
    break;
  case 'q':
    return true;
  default:
    goto get_user_input;
  }

  // Hide the tooltip below the map.
  for (int x = screen->offset.x; x < screen->ws.ws_col; x += 2) {
    const int y = screen->offset.y + screen->map_height + 2;
    print(y, x, "  ");
  }

  nonblocking_input(true);
  return false;
}

static void update_doodle(struct snake *doodle, const struct point dialog_begin,
                          const int dialog_height, const int dialog_width) {
  doodle->old_tail = doodle->body[0];
  for (size_t i = 0; i < doodle->length - 1; ++i) {
    doodle->body[i] = doodle->body[i + 1];
  }

  // Head moves forward
  struct point head = doodle->body[doodle->length - 1];
  switch (doodle->direction) {
  case UP:
    if (head.y >= dialog_begin.y) {
      --head.y;
      break;
    }
    doodle->direction = LEFT;
    // fallthrough
  case LEFT:
    if (head.x > dialog_begin.x) {
      head.x -= 2;
      break;
    }
    doodle->direction = DOWN;
    // fallthrough
  case DOWN:
    if (head.y - 1 < dialog_begin.y + dialog_height) {
      ++head.y;
      break;
    }
    doodle->direction = RIGHT;
    // fallthrough
  case RIGHT:
    if (head.x < dialog_begin.x + dialog_width - 1) {
      head.x += 2;
      break;
    }
    doodle->direction = UP;
    --head.y;
  }
  doodle->body[doodle->length - 1] = head;

  set_color(BRIGHT_GREEN);
  print(head.y, head.x, "██");
  if (doodle->length >= 2) {
    set_color(GREEN);
    const struct point pre_head = doodle->body[doodle->length - 2];
    print(pre_head.y, pre_head.x, "██");
  }
  print(doodle->old_tail.y, doodle->old_tail.x, "  ");

  nanosleep(&(struct timespec){0, 33333333}, NULL);
}

bool screen_dialog(struct screen *self, enum dialog_kind kind,
                   enum difficulty *difficulty, const unsigned score) {
  static const int dialog_height = 16, dialog_width = 57;
  const struct point begin = {
      self->offset.x + self->map_width - dialog_width / 2 + 1,
      self->offset.y + self->map_height / 2 - dialog_height / 2 + 1};

  // Add right offset so that changing difficulty doesn't interfere with doodle
  const int difficulty_offset_x = begin.x + (kind == WELCOME ? 3 : 0);

  struct snake *doodle = NULL; // snake decoration on welcome screen
  const char **fmt = NULL;     // Select the appropriate format string
  switch (kind) {
  case WELCOME:
    fmt = welcome;
    doodle = snake_create((struct point){begin.x, begin.y + 2}, 7);
    doodle->direction = DOWN;
    set_color(GREEN);
    for (int i = 0; i < 7; ++i) { // Make it long 7
      doodle->body[doodle->length] =
          (struct point){begin.x, doodle->body[doodle->length - 1].y + 1};
      ++doodle->length;
      print(doodle->body[doodle->length - 1].y,
            doodle->body[doodle->length - 1].x, "██");
    }
    break;
  case OVER:
    fmt = over;
    // Hide score above the map
    for (int x = self->offset.x; x < self->ws.ws_col; x += 2) {
      print(self->offset.y - 2, x, "  ");
    }
    nonblocking_input(false);
    break;
  case WIN:
    fmt = win;
    nonblocking_input(false);
    break;
  }

  // Draw the dialog
  set_color(DEFAULT_COLOR);
  for (int y = begin.y, i = 0; y < begin.y + dialog_height; ++y, ++i) {
    if (kind != WELCOME && i == 9) { // Plug in the score
      print(y, begin.x, fmt[i], score);
    } else if (i == 11) { // Plug in the difficulty
      print(y, difficulty_offset_x, fmt[i], diff[*difficulty]);
    } else {
      print(y, begin.x, fmt[i]);
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
    case ARROW_RIGHT: // increment difficulty
      if (kind != WIN && *difficulty != HARD) {
        ++*difficulty;
        set_color(DEFAULT_COLOR);
        print(begin.y + 11, difficulty_offset_x, fmt[11], diff[*difficulty]);
      }
      break;
    case '<':
    case ARROW_LEFT: // decrement difficulty
      if (kind != WIN && *difficulty != INCREMENTAL) {
        --*difficulty;
        set_color(DEFAULT_COLOR);
        print(begin.y + 11, difficulty_offset_x, fmt[11], diff[*difficulty]);
      }
      break;
    case 'n':
    case 'q':
      snake_destroy(doodle);
      return true;
    }

    if (kind == WELCOME) {
      update_doodle(doodle, begin, dialog_height, dialog_width);
    }
  }
}
