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

#include <locale.h>
#include <ncurses.h>
#include <stdbool.h>
#include <stdlib.h>
#include <threads.h>

#include "screen.h"
#include "snake.h"

Screen *newScreen(void) {
  Screen *self = malloc(sizeof(Screen));

  self->width = getmaxx(stdscr) - 1;
  self->height = getmaxy(stdscr) - 1;
  self->mapWidth = self->width / 4; // Further down is explained why 4
  self->mapHeight = self->height * 2 / 3;

  self->offset = (Point){(self->width - self->mapWidth * 2) / 2,
                         (self->height - self->mapHeight) / 2};

  self->grid = malloc(sizeof(int * [self->mapHeight + 1]));
  for (int i = 0; i <= self->mapHeight; ++i)
    self->grid[i] = calloc(self->mapWidth + 1, sizeof(int));

  return self;
}

void destroyScreen(Screen *self) {
  if (self != NULL) {
    if (self->grid != NULL) {
      for (int i = 0; i <= self->mapHeight; ++i)
        free(self->grid[i]);
      free(self->grid);
    }
    free(self);
    self = NULL;
  }
}

void initializeNcurses(void) {
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
}

// Translate an x coordinate to display on the Screen.
// This is because two cells are used to display one point: "██". When handling
// widths and x coordinates half as many as there are on the screen are
// considered. So when it comes to representing those coordinates it is as if
// the screen is _one cell yes, the next no, one cell yes, the next no..._
// █ █ █ █ █ █. To represent x = 4 on the screen x must become 9
static int translate(const int x) { return x + x + 1; }

// Color is one of the eight terminal colors provided by ncurses.
// COLOR_BLACK = 0 is the current fg color (i.e. actual WHITE)
static void setColor(const int color) {
  init_pair(color, color, -1);
  attrset(COLOR_PAIR(color));
}

void drawPoint(const Screen *self, const Point pos, const int color) {
  setColor(color);
  mvprintw(pos.y + self->offset.y, translate(pos.x) + self->offset.x, "██");
}

bool insideBoundaries(const Screen *self, const Snake *snake) {
  return snake->head->pos.x <= self->mapWidth && snake->head->pos.x >= 0 &&
         snake->head->pos.y <= self->mapHeight && snake->head->pos.y >= 0;
}

void spawnOrb(Screen *self) {
  do {
    self->orb.x = rand() % (self->mapWidth + 1);
    self->orb.y = rand() % (self->mapHeight + 1);
  } while (self->grid[self->orb.y][self->orb.x] == 1);

  drawPoint(self, self->orb, COLOR_MAGENTA);
}

void updateScore(const Screen *self, const unsigned score) {
  setColor(0);
  mvprintw(self->offset.y - 2, self->offset.x, "Score: %d", score);
}

void drawWalls(const Screen *self) {
  erase(); // Clean the terminal
  setColor(COLOR_YELLOW);

  const Point northWest = {self->offset.x, self->offset.y - 1},
              southEasth = {translate(self->mapWidth) + self->offset.x + 2,
                            self->mapHeight + self->offset.y + 1};

  for (int i = northWest.x; i <= southEasth.x; ++i) {
    mvprintw(northWest.y, i, "▄");
    mvprintw(southEasth.y, i, "▀");
  }
  for (int j = northWest.y + 1; j < southEasth.y; ++j) {
    mvprintw(j, northWest.x, "█");
    mvprintw(j, southEasth.x, "█");
  }
}

void draw(const Screen *self, Snake *snake) {
  // Cover the old tail with a blank if the Snake has not grown
  if (!snake->growing) {
    mvprintw(snake->oldTail.y + self->offset.y,
             translate(snake->oldTail.x) + self->offset.x, "  ");
    self->grid[snake->oldTail.y][snake->oldTail.x] = 0;
  }

  // Draw the new head added by Snake::advance()
  drawPoint(self, snake->head->pos, 8);
  if (snake->head->prev != NULL)
    drawPoint(self, snake->head->prev->pos, COLOR_GREEN);
  self->grid[snake->head->pos.y][snake->head->pos.x] = 1;
}

bool prepareGame(Screen *self, Snake *snake) {
  drawWalls(self);
  spawnOrb(self);
  updateScore(self, snake->length);
  drawPoint(self, snake->head->pos, 8); // Draw the head of the snake
  setColor(0);                          // Tip at the bottom
  mvprintw(self->offset.y + self->mapHeight + 2, self->offset.x,
           "Move in any direction to start the game.");

  nodelay(stdscr, false);
  // Get the initial direction of the snake
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
  }

  mvhline(self->offset.y + self->mapHeight + 2, self->offset.x, ' ',
          self->width); // Hide tip at the bottom
  nodelay(stdscr, true);   // getch() doesn't wait for input
  return false;
}

// Move the little green snake on the welcome screen
static void updateDoodle(Snake *doodle, const Point beginDialog,
                         const int dialogHeight, const int dialogWidth) {
  doodle->oldTail = doodle->tail->pos;
  ouroboros(doodle); // Tail becomes the head

  // Head moves forward
  switch (doodle->direction) {
  case NORTH:
    if (doodle->head->pos.y >= beginDialog.y) {
      --doodle->head->pos.y;
      break;
    }
    doodle->direction = WEST;
    [[fallthrough]];
  case WEST:
    if (doodle->head->pos.x > beginDialog.x) {
      doodle->head->pos.x -= 2;
      break;
    }
    doodle->direction = SOUTH;
    [[fallthrough]];
  case SOUTH:
    if (doodle->head->pos.y - 1 < beginDialog.y + dialogHeight) {
      ++doodle->head->pos.y;
      break;
    }
    doodle->direction = EAST;
    [[fallthrough]];
  case EAST:
    if (doodle->head->pos.x < beginDialog.x + dialogWidth - 1) {
      doodle->head->pos.x += 2;
      break;
    }
    doodle->direction = NORTH;
    --doodle->head->pos.y;
  }

  // Draw the head, hide the old tail, and sleep
  setColor(8);
  mvprintw(doodle->head->pos.y, doodle->head->pos.x, "██");
  if (doodle->head->prev != NULL) {
    setColor(COLOR_GREEN);
    mvprintw(doodle->head->prev->pos.y, doodle->head->prev->pos.x, "██");
  }
  mvprintw(doodle->oldTail.y, doodle->oldTail.x, "  ");
  thrd_sleep(&(const struct timespec){0, 33333333L}, NULL); // 30 fps
}

bool dialog(Screen *self, Difficulty *difficulty, const bool gameOver,
            const unsigned score, const Point collision) {
  static const char
      *fmtDifficulty[] = {"  incremental >", "   < easy >    ",
                          "  < medium >   ", "   < hard      "},
      *fmtWelcome[] = {"",
                       "                              _",
                       "                             | |",
                       "              ___ _ __   __ _| | _____   ___",
                       "             / __| '_ \\ / _` | |/ / _ \\ / __|",
                       "             \\__ \\ | | | (_| |   <  __/| (__",
                       "             |___/_| |_|\\__,_|_|\\_\\___(_)___|",
                       "",
                       "                          Play ⏎",
                       "",
                       "             Difficulty: %s",
                       "",
                       "                          Quit q",
                       "",
                       "          by Mario D'Andrea <https://ormai.dev>",
                       ""},
      *fmtGameOver[] = {
          "┏━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┓",
          "┃   _____                        _____                  ┃",
          "┃  |  __ \\                      |  _  |                 ┃",
          "┃  | |  \\/ __ _ _ __ ___   ___  | | | |_   _____ _ __   ┃",
          "┃  | | __ / _` | '_ ` _ \\ / _ \\ | | | \\ \\ / / _ \\ '__|  ┃",
          "┃  | |_\\ \\ (_| | | | | | |  __/ \\ \\_/ /\\ V /  __/ |     ┃",
          "┃   \\____/\\__,_|_| |_| |_|\\___|  \\___/  \\_/ \\___|_|     ┃",
          "┃                                                       ┃",
          "┃                  Your score was: %-4d                 ┃",
          "┃                                                       ┃",
          "┃              Difficulty: %s              ┃",
          "┃                                                       ┃",
          "┃                      Play again?                      ┃",
          "┃                          y/n                          ┃",
          "┃                                                       ┃",
          "┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛"};

  const char **fmt = gameOver ? fmtGameOver : fmtWelcome;

  if (gameOver) {
    if (collision.x != -1 && collision.y != -1)
      drawPoint(self, collision, COLOR_RED);

    // Hide score count above the playing field
    mvhline(self->offset.y - 2, self->offset.x - 1, ' ', self->width);
    nodelay(stdscr, false); // to avoid 100% CPU usage
  }

  static const int dialogHeight = 16, dialogWidth = 57;
  const Point begin = {self->offset.x + self->mapWidth - dialogWidth / 2 + 1,
                       self->offset.y + self->mapHeight / 2 - dialogHeight / 2 +
                           1};
  const int diffFmtX = begin.x + (gameOver ? 0 : 3);

  setColor(0);
  for (int y = begin.y, i = 0; y < begin.y + dialogHeight; ++y, ++i)
    if (gameOver && i == 8) // Plug in the score
      mvprintw(y, begin.x, fmt[i], score);
    else if (i == 10) // Plug in the difficulty
      mvprintw(y, diffFmtX, fmt[i], fmtDifficulty[*difficulty]);
    else
      mvprintw(y, begin.x, "%s", fmt[i]);

  Snake *doodle = NULL;
  if (!gameOver) {
    doodle = newSnake((Point){begin.x, begin.y + 2});
    doodle->direction = SOUTH;
    setColor(COLOR_GREEN);
    for (int i = 0; i < 7; ++i) {
      doodle->head->next =
          newNode((Point){begin.x, doodle->head->pos.y + 1}, doodle->head);
      doodle->head = doodle->head->next;
      mvprintw(doodle->head->pos.y, doodle->head->pos.x, "██");
    }
  }

  while (true) { // Listen for keyboard input
    switch (getch()) {
    case '\n':
    case 'y': {
      destroySnake(doodle);
      return false;
    }
    case '>':
    case KEY_RIGHT: // increment difficulty
      if (*difficulty != HARD) {
        ++*difficulty;
        setColor(0);
        mvprintw(begin.y + 10, diffFmtX, fmt[10], fmtDifficulty[*difficulty]);
      }
      break;
    case '<':
    case KEY_LEFT: // decrement difficulty
      if (*difficulty != INCREMENTAL) {
        --*difficulty;
        setColor(0);
        mvprintw(begin.y + 10, diffFmtX, fmt[10], fmtDifficulty[*difficulty]);
      }
      break;
    case 'n':
    case 'q':
      destroySnake(doodle);
      return true;
    }

    if (!gameOver)
      updateDoodle(doodle, begin, dialogHeight, dialogWidth);
  }
}
