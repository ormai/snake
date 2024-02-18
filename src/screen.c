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

  self->grid = malloc((self->height + 1) * sizeof(int *));
  for (int i = 0; i <= self->height; ++i)
    self->grid[i] = calloc(self->width + 1, sizeof(int));

  self->difficulty = INCREMENTAL;
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
  cbreak(); // Disable keybord input buffering, keys are immediately evaluated
  noecho(); // Disabl echoing for getch()
  intrflush(stdscr, false); // Flush the tty on quit
  keypad(stdscr, true);     // Enable keypad for the arrow keys
  curs_set(0);              // Make the cursor invisible
  start_color();            // Have some colors
  use_default_colors();
}

static const char *difficultyFmt[] = {"  incremental >", "   < easy >    ",
                                      "  < medium >   ", "   < hard      "};
bool welcome(Screen *self) {
  static const char *fmt[] = {
      "┏━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┓",
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
      "┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛"};

  const int fmtHeight = 16, fmtWidth = 57;
  const Point begin = {self->offset.x + self->width - fmtWidth / 2,
                       self->offset.y + self->height / 2 - fmtHeight / 2};

  for (int y = begin.y, i = 0; y < begin.y + fmtHeight; ++y, ++i)
    if (i == 10) // Plug in the difficulty
      mvprintw(y, begin.x, fmt[i], difficultyFmt[self->difficulty]);
    else
      mvprintw(y, begin.x, "%s", fmt[i]);

  while (true) {
    switch (getch()) {
    case '\n':
    case 'p':
      drawWalls(self);
      nodelay(stdscr, true); // getch() doesn't wait for input
      return true;
    case KEY_RIGHT: // increment difficulty
      if (self->difficulty != HARD) {
        ++self->difficulty;
        mvprintw(begin.y + 10, begin.x, fmt[10],
                 difficultyFmt[self->difficulty]);
      }
      break;
    case KEY_LEFT: // decrement difficulty
      if (self->difficulty != INCREMENTAL) {
        --self->difficulty;
        mvprintw(begin.y + 10, begin.x, fmt[10],
                 difficultyFmt[self->difficulty]);
      }
      break;
    case 'q':
      return false;
    }
  }
}

// Take an x coordinate and transate it for the screen
// This is due to the fact that thare a point on th escree is two character wide
// This is just for the representation
static int translate(const int x) { return x + x + 1; }

// color is one of the colors provided by ncurses
void drawPointWithColor(const Screen *self, const Point pos, const int color) {
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

  drawPointWithColor(self, self->orb, COLOR_MAGENTA);
}

void updateScore(const Screen *self, const unsigned score) {
  mvprintw(self->offset.y - 2, self->offset.x, "Score: %d", score);
}

void drawWalls(const Screen *self) {
  erase();

  const Point topLeftCorner = {self->offset.x, self->offset.y},
              bottomRightCorner = {translate(self->width) + self->offset.x,
                                   self->height + self->offset.y};

  mvprintw(topLeftCorner.y - 1, topLeftCorner.x, "╔");
  mvprintw(topLeftCorner.y - 1, bottomRightCorner.x + 2, "╗");
  mvprintw(bottomRightCorner.y + 1, topLeftCorner.x, "╚");
  mvprintw(bottomRightCorner.y + 1, bottomRightCorner.x + 2, "╝");

  for (int i = topLeftCorner.x + 1; i <= bottomRightCorner.x + 1; ++i) {
    mvprintw(topLeftCorner.y - 1, i, "═");
    mvprintw(bottomRightCorner.y + 1, i, "═");
  }
  for (int j = topLeftCorner.y; j <= bottomRightCorner.y; ++j) {
    mvprintw(j, topLeftCorner.x, "║");
    mvprintw(j, bottomRightCorner.x + 2, "║");
  }
}

void draw(const Screen *self, const Snake *snake, const bool growing,
          const Node *oldTail) {
  // Draw the new head added by Snake::advance()
  drawPointWithColor(self, snake->head->pos, COLOR_GREEN);
  self->grid[snake->head->pos.y][snake->head->pos.x] = 1;

  // Cover the old tail with a blank if the Snake has not grown
  if (!growing) {
    mvprintw(oldTail->pos.y + self->offset.y,
             translate(oldTail->pos.x) + self->offset.x, "  ");
    self->grid[oldTail->pos.y][oldTail->pos.x] = 0;
  }
}

bool gameOver(Screen *self, Snake *snake, Point *collision, float *progress) {
  if (collision->x != -1 && collision->y != -1)
    drawPointWithColor(self, *collision, COLOR_RED);

  // hide score count above the playing field
  mvhline(self->offset.y - 2, self->offset.x - 1, ' ', self->width);

  static const char *fmt[] = {
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

  const int fmtHeight = 16, fmtWidth = 57;
  const Point begin = {self->offset.x + self->width - fmtWidth / 2,
                       self->offset.y + self->height / 2 - fmtHeight / 2};

  for (int y = begin.y, i = 0; y < begin.y + fmtHeight; ++y, ++i)
    if (i == 8) // Plug in the score
      mvprintw(y, begin.x, fmt[i], snake->length);
    else if (i == 10) // Plug in the difficulty
      mvprintw(y, begin.x, fmt[i], difficultyFmt[self->difficulty]);
    else
      mvprintw(y, begin.x, "%s", fmt[i]);

  Difficulty difficulty = self->difficulty; // remember previous difficulty

  nodelay(stdscr, false);
  while (true) {
    switch (getch()) {
    case '\n':
    case 'y': {
      // Reset the game
      destroyScreen(self);
      destroySnake(snake);

      self = newScreen();
      self->difficulty = difficulty;
      snake = newSnake((Point){self->width / 2, self->height / 2});

      drawWalls(self);
      *collision = (Point){-1, -1};
      *progress = 0.0;
      spawnOrb(self);
      updateScore(self, snake->length);
      nodelay(stdscr, true);
      return true;
    }
    case KEY_RIGHT: // increment difficulty
      if (difficulty != HARD) {
        ++difficulty;
        mvprintw(begin.y + 10, begin.x, fmt[10], difficultyFmt[difficulty]);
      }
      break;
    case KEY_LEFT: // decrement difficulty
      if (difficulty != INCREMENTAL) {
        --difficulty;
        mvprintw(begin.y + 10, begin.x, fmt[10], difficultyFmt[difficulty]);
      }
      break;
    case 'n':
    case 'q':
      return false;
    }
  }
}
