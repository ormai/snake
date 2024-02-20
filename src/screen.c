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

#include <ncurses.h>
#include <stdlib.h>

#include "screen.h"

Screen *newScreen(void) {
  Screen *self = malloc(sizeof(Screen));

  self->screenWidth = getmaxx(stdscr) - 1;
  self->screenHeight = getmaxy(stdscr) - 1;
  self->width = self->screenWidth / 4;
  self->height = self->screenHeight * 2 / 3;

  self->offset = (Point){(self->screenWidth - self->width * 2) / 2,
                         (self->screenHeight - self->height) / 2};

  self->grid = malloc(sizeof(int * [self->height + 1]));
  for (int i = 0; i <= self->height; ++i)
    self->grid[i] = calloc(self->width + 1, sizeof(int));
  return self;
}

void destroyScreen(Screen *self) {
  if (self != NULL) {
    if (self->grid != NULL) {
      for (int i = 0; i <= self->height; ++i)
        free(self->grid[i]);
      free(self->grid);
    }
    free(self);
    self = NULL;
  }
}

void initializeNcurses(void) {
  initscr();
  cbreak(); // Disable keyboard input buffering, keys are immediately evaluated
  noecho(); // Disable echoing for getch()
  intrflush(stdscr, false); // Flush the tty on quit
  keypad(stdscr, true);     // Enable keypad for the arrow keys
  curs_set(0);              // Make the cursor invisible
  start_color();            // Have some colors
  use_default_colors();
}

// Take an x coordinate and transate it for the screen
// This is due to the fact that a point on thee screen is two character wide
// This is just for the representation
static inline int translate(const int x) { return x + x + 1; }

// color is one of the colors provided by ncurses
void drawPoint(const Screen *self, const Point pos, const int color) {
  init_pair(color, color, -1);
  attrset(COLOR_PAIR(color)); // Set color

  mvprintw(pos.y + self->offset.y, translate(pos.x) + self->offset.x, "██");

  init_pair(7, COLOR_WHITE, -1);
  attrset(COLOR_PAIR(7)); // Restore white color
}

bool insideBoundaries(const Screen *self, const Snake *snake) {
  return snake->head->pos.x <= self->width && snake->head->pos.x >= 0 &&
         snake->head->pos.y <= self->height && snake->head->pos.y >= 0;
}

void spawnOrb(Screen *self) {
  do {
    self->orb.x = rand() % (self->width + 1);
    self->orb.y = rand() % (self->height + 1);
  } while (self->grid[self->orb.y][self->orb.x] == 1);

  drawPoint(self, self->orb, COLOR_MAGENTA);
}

void updateScore(const Screen *self, const unsigned score) {
  mvprintw(self->offset.y - 2, self->offset.x, "Score: %d", score);
}

void drawWalls(const Screen *self) {
  erase();

  const Point northWest = {self->offset.x, self->offset.y},
              southEasth = {translate(self->width) + self->offset.x,
                            self->height + self->offset.y};

  mvprintw(northWest.y - 1, northWest.x, "╔");
  mvprintw(northWest.y - 1, southEasth.x + 2, "╗");
  mvprintw(southEasth.y + 1, northWest.x, "╚");
  mvprintw(southEasth.y + 1, southEasth.x + 2, "╝");

  for (int i = northWest.x + 1; i <= southEasth.x + 1; ++i) {
    mvprintw(northWest.y - 1, i, "═");
    mvprintw(southEasth.y + 1, i, "═");
  }
  for (int j = northWest.y; j <= southEasth.y; ++j) {
    mvprintw(j, northWest.x, "║");
    mvprintw(j, southEasth.x + 2, "║");
  }
}

void draw(const Screen *self, const Snake *snake, const bool growing,
          const Node *oldTail) {
  // Draw the new head added by Snake::advance()
  drawPoint(self, snake->head->pos, COLOR_GREEN);
  self->grid[snake->head->pos.y][snake->head->pos.x] = 1;

  // Cover the old tail with a blank if the Snake has not grown
  if (!growing) {
    mvprintw(oldTail->pos.y + self->offset.y,
             translate(oldTail->pos.x) + self->offset.x, "  ");
    self->grid[oldTail->pos.y][oldTail->pos.x] = 0;
  }
}

// if parameter gameOver is false, score and collision are irrelevant
bool dialog(Screen *self, Difficulty *difficulty, const bool gameOver,
            const unsigned score, const Point collision) {
  static const char
      *fmtDifficulty[] = {"  incremental >", "   < easy >    ",
                          "  < medium >   ", "   < hard      "},
      *fmtWelcome[] =
          {"┏━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┓",
           "┃                             _                         ┃",
           "┃                            | |                        ┃",
           "┃             ___ _ __   __ _| | _____   ___            ┃",
           "┃            / __| '_ \\ / _` | |/ / _ \\ / __|           ┃",
           "┃            \\__ \\ | | | (_| |   <  __/| (__            ┃",
           "┃            |___/_| |_|\\__,_|_|\\_\\___(_)___|           ┃",
           "┃                                                       ┃",
           "┃                         Play ⏎                        ┃",
           "┃                                                       ┃",
           "┃              Difficulty: %s              ┃",
           "┃                                                       ┃",
           "┃                         Quit q                        ┃",
           "┃                                                       ┃",
           "┃         by Mario D'Andrea <https://ormai.dev>         ┃",
           "┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛"},
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

    // hide score count above the playing field
    mvhline(self->offset.y - 2, self->offset.x - 1, ' ', self->width);
    nodelay(stdscr, false);
  }

  static const int dialogHeight = 16, dialogWidth = 57;
  const Point begin = {self->offset.x + self->width - dialogWidth / 2 + 1,
                       self->offset.y + self->height / 2 - dialogHeight / 2 +
                           1};

  for (int y = begin.y, i = 0; y < begin.y + dialogHeight; ++y, ++i)
    if (gameOver && i == 8) // Plug in the score
      mvprintw(y, begin.x, fmt[i], score);
    else if (i == 10) // Plug in the difficulty
      mvprintw(y, begin.x, fmt[i], fmtDifficulty[*difficulty]);
    else
      mvprintw(y, begin.x, "%s", fmt[i]);

  while (true) {
    switch (getch()) {
    case '\n':
    case 'y': {
      nodelay(stdscr, true); // getch() doesn't wait for input
      return false;
    }
    case '>':
    case KEY_RIGHT: // increment difficulty
      if (*difficulty != HARD) {
        ++*difficulty;
        mvprintw(begin.y + 10, begin.x, fmt[10], fmtDifficulty[*difficulty]);
      }
      break;
    case '<':
    case KEY_LEFT: // decrement difficulty
      if (*difficulty != INCREMENTAL) {
        --*difficulty;
        mvprintw(begin.y + 10, begin.x, fmt[10], fmtDifficulty[*difficulty]);
      }
      break;
    case 'n':
    case 'q':
      return true;
    }
  }
}
