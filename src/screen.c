/* screen.c -- Implementation of the class Screen
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

#define _POSIX_C_SOURCE 199309L

#include <locale.h>
#include <ncurses.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>
#include <wchar.h>

#include "screen.h"
#include "snake.h"

Screen *screen_create(void) {
  Screen *self = malloc(sizeof(Screen));

  self->width = getmaxx(stdscr) - 1;
  self->height = getmaxy(stdscr) - 1;
  self->map_width = self->width / 4; // Further down is explained why 4
  self->map_height = self->height * 2 / 3;
  self->playing_surface = self->map_width * self->map_height;

  self->offset = (Point){(self->width - self->map_width * 2) / 2,
                         (self->height - self->map_height) / 2};

  self->grid = malloc(sizeof(int * [self->map_height + 1]));
  for (int i = 0; i <= self->map_height; ++i) {
    self->grid[i] = calloc(self->map_width + 1, sizeof(int));
  }

  return self;
}

void screen_destroy(Screen *self) {
  if (self != NULL) {
    if (self->grid != NULL) {
      for (size_t i = 0; i <= self->map_height; ++i) {
        free(self->grid[i]);
      }
      free(self->grid);
    }
    free(self);
    self = NULL;
  }
}

void screen_prepare(void) {
  setlocale(LC_ALL, ""); // Use the locale of the environment
  initscr();
  cbreak(); // Disable keyboard input buffering, keys are immediately evaluated
  noecho(); // Disable echoing for getch()
  intrflush(stdscr, false); // Flush the tty on quit
  keypad(stdscr, true);     // Enable keypad for the arrow keys
  nodelay(stdscr, true);    // getch() doesn't wait for input
  curs_set(0);              // Make the cursor invisible
  start_color();            // Have some colors
  use_default_colors();
  init_color(8, 721, 733, 149); // #B8BB26 color for the head of the Snake
}

// Translate an x coordinate to display on the Screen.
// This is because two cells are used to display one point: "██". When handling
// widths and x coordinates half as many cells as there are on the screen are
// considered. So when it comes to representing those coordinates it is as if
// the screen is _one cell yes, the next no, one cell yes, the next no..._
// █ █ █ █ █ █. To represent x = 4 on the screen x must become 9.
static int translate(const int x) { return x + x + 1; }

// Color is one of the eight terminal colors provided by ncurses.
// COLOR_BLACK = 0 is the current fg color (i.e. actual WHITE)
static void set_color(const int color) {
  init_pair(color, color, -1);
  attrset(COLOR_PAIR(color));
}

void screen_draw_point(const Screen *self, const Point pos, const int color) {
  set_color(color);
  mvprintw(pos.y + self->offset.y, translate(pos.x) + self->offset.x, "██");
}

bool screen_inside_boundaries(const Screen *self, const Snake *snake) {
  return snake->head->pos.x <= self->map_width && snake->head->pos.x >= 0 &&
         snake->head->pos.y <= self->map_height && snake->head->pos.y >= 0;
}

void screen_spawn_orb(Screen *self) {
  // This is a critical point. With a big enough map and when the Snake is
  // short there is no problem. But when progressing towards the completion of
  // the game the app will probably stall, trying to randomly get a correct
  // position for the orb. One solution I thought is creating a dynamic
  // structure that holds the set of current available Points to choose from to
  // spawn a new orb. But this is a lot of code and could slow things down
  // anyway. So I will leave the problem open for now.
  do {
    self->orb.x = rand() % (self->map_width + 1);
    self->orb.y = rand() % (self->map_height + 1);
  } while (self->grid[self->orb.y][self->orb.x] == 1);

  screen_draw_point(self, self->orb, COLOR_MAGENTA);
}

void screen_update_score(const Screen *self, const unsigned score) {
  set_color(0);
  mvprintw(self->offset.y - 2, self->offset.x, "Score: %d", score);
}

void screen_draw_walls(const Screen *self) {
  erase(); // Clean the terminal
  set_color(COLOR_YELLOW);

  const Point northWest = {self->offset.x, self->offset.y - 1},
              southEasth = {translate(self->map_width) + self->offset.x + 2,
                            self->map_height + self->offset.y + 1};

  // Not able to use mvhline() and mvvline() because of the wide characters
  for (int i = northWest.x; i <= southEasth.x; ++i) {
    mvprintw(northWest.y, i, "▄");
    mvprintw(southEasth.y, i, "▀");
  }
  for (int j = northWest.y + 1; j < southEasth.y; ++j) {
    mvprintw(j, northWest.x, "█");
    mvprintw(j, southEasth.x, "█");
  }
}

void screen_draw(const Screen *self, Snake *snake) {
  // Cover the old tail with a blank if the Snake has not grown
  if (!snake->growing) {
    mvprintw(snake->old_tail.y + self->offset.y,
             translate(snake->old_tail.x) + self->offset.x, "  ");
    self->grid[snake->old_tail.y][snake->old_tail.x] = 0; // mark it free
  }

  // Draw the new head added by Snake::advance()
  screen_draw_point(self, snake->head->pos, 8);
  if (snake->head->prev != NULL) {
    screen_draw_point(self, snake->head->prev->pos, COLOR_GREEN);
  }
  self->grid[snake->head->pos.y][snake->head->pos.x] = 1; // mark it occupied
}

bool screen_prepare_game(Screen *self, Snake *snake) {
  screen_draw_walls(self);
  screen_spawn_orb(self);
  screen_update_score(self, snake->length);
  screen_draw_point(self, snake->head->pos, 8); // Draw the head of the snake
  set_color(0);                                 // Tip at the bottom
  mvprintw(self->offset.y + self->map_height + 2, self->offset.x,
           "Move in any direction to start the game.");

  nodelay(stdscr, false);

get_user_input: // Get the initial direction of the snake
  switch (getch()) {
  case 'w':
  case 'k':
  case KEY_UP:
    snake->direction = NORTH;
    break;
  case 'l':
  case 'd':
  case KEY_RIGHT:
    snake->direction = EAST;
    break;
  case 'j':
  case 's':
  case KEY_DOWN:
    snake->direction = SOUTH;
    break;
  case 'h':
  case 'a':
  case KEY_LEFT:
    snake->direction = WEST;
    break;
  case 'q':
    return true;
  default:
    goto get_user_input;
  }

  mvhline(self->offset.y + self->map_height + 2, self->offset.x, ' ',
          self->width);  // Hide suggestion at the bottom
  nodelay(stdscr, true); // getch() doesn't wait for input
  return false;
}

static void update_doodle(Snake *doodle, const Point dialog_begin,
                          const int dialog_height, const int dialog_width) {
  doodle->old_tail = doodle->tail->pos;
  snake_ouroboros(doodle); // Tail becomes the head

  // Head moves forward
  switch (doodle->direction) {
  case NORTH:
    if (doodle->head->pos.y >= dialog_begin.y) {
      --doodle->head->pos.y;
      break;
    }
    doodle->direction = WEST;
    /* fallthrough */
  case WEST:
    if (doodle->head->pos.x > dialog_begin.x) {
      doodle->head->pos.x -= 2;
      break;
    }
    doodle->direction = SOUTH;
    /* fallthrough */
  case SOUTH:
    if (doodle->head->pos.y - 1 < dialog_begin.y + dialog_height) {
      ++doodle->head->pos.y;
      break;
    }
    doodle->direction = EAST;
    /* fallthrough */
  case EAST:
    if (doodle->head->pos.x < dialog_begin.x + dialog_width - 1) {
      doodle->head->pos.x += 2;
      break;
    }
    doodle->direction = NORTH;
    --doodle->head->pos.y;
  }

  // Draw the head, hide the old tail, and sleep
  set_color(8);
  mvprintw(doodle->head->pos.y, doodle->head->pos.x, "██");
  if (doodle->head->prev != NULL) {
    set_color(COLOR_GREEN);
    mvprintw(doodle->head->prev->pos.y, doodle->head->prev->pos.x, "██");
  }
  mvprintw(doodle->old_tail.y, doodle->old_tail.x, "  ");
  nanosleep(&(struct timespec){0, 33333333}, NULL);
}

bool screen_dialog(Screen *self, DialogKind kind, Difficulty *difficulty,
                   const unsigned score, const Point collision) {
  static const int dialog_height = 16, dialog_width = 57;
  const Point begin = {self->offset.x + self->map_width - dialog_width / 2 + 1,
                       self->offset.y + self->map_height / 2 -
                           dialog_height / 2 + 1};
  static char
      *diff[] = {"  incremental >", "   < easy >    ", "  < medium >   ",
                 "   < hard      "},
      *welcome[] = {"",
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
                    ""},
      *over[] =
          {"┏━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┓",
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
           "┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛"},
      *win[] = {
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

  Snake *doodle = NULL; // Snake decoration on welcome screen
  char **fmt = NULL;    // Select the appropriate format string

  // Add right offset so that changing difficulty doesn't interfere with doodle
  const int difficulty_offset_x = begin.x + (kind == WELCOME ? 3 : 0);

  switch (kind) {
  case WELCOME:
    fmt = welcome;
    doodle = snake_create((Point){begin.x, begin.y + 2});
    doodle->direction = SOUTH;
    set_color(COLOR_GREEN);
    for (int i = 0; i < 7; ++i) { // Make it long 7
      doodle->head->next =
          node_create((Point){begin.x, doodle->head->pos.y + 1}, doodle->head);
      doodle->head = doodle->head->next;
      mvprintw(doodle->head->pos.y, doodle->head->pos.x, "██");
    }
    break;
  case OVER:
    fmt = over;
    if (collision.x != -1 && collision.y != -1) {
      screen_draw_point(self, collision, COLOR_RED);
    }
    // Hide score count above the playing field
    mvhline(self->offset.y - 2, self->offset.x - 1, ' ', self->width);
    nodelay(stdscr, false);
    break;
  case WIN:
    fmt = win;
    nodelay(stdscr, false);
    break;
  }

  // Draw the dialog
  set_color(0);
  for (int y = begin.y, i = 0; y < begin.y + dialog_height; ++y, ++i) {
    if (kind != WELCOME && i == 9) { // Plug in the score
      mvprintw(y, begin.x, fmt[i], score);
    } else if (i == 11) { // Plug in the difficulty
      mvprintw(y, difficulty_offset_x, fmt[i], diff[*difficulty]);
    } else {
      mvprintw(y, begin.x, "%s", fmt[i]);
    }
  }

  while (true) { // Listen for keyboard input
    switch (getch()) {
    case '\n':
    case 'y': {
      snake_destroy(doodle);
      return false;
    }
    case '>':
    case KEY_RIGHT: // increment difficulty
      if (kind != WIN && *difficulty != HARD) {
        ++*difficulty;
        set_color(0);
        mvprintw(begin.y + 11, difficulty_offset_x, fmt[11], diff[*difficulty]);
      }
      break;
    case '<':
    case KEY_LEFT: // decrement difficulty
      if (kind != WIN && *difficulty != INCREMENTAL) {
        --*difficulty;
        set_color(0);
        mvprintw(begin.y + 11, difficulty_offset_x, fmt[11], diff[*difficulty]);
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
