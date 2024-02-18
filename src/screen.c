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

  self->grid = calloc(self->width * self->height, sizeof(bool));
  spawnOrb(self);

  return self;
}

void destroyScreen(Screen *self) {
  if (self != NULL) {
    if (self->grid != NULL)
      free(self->grid);
    free(self);
  }
}

void initializeNcurses(void) {
  initscr();
  cbreak(); // Disable keybord input buffering, keys are immediately evaluated
  noecho(); // Disabl echoing for getch()
  intrflush(stdscr, false); // Flush the tty on quit
  keypad(stdscr, true);     // Enable keypad for the arrow keys
  nodelay(stdscr, true);    // getch() doesn't wait for input
  curs_set(0);              // Make the cursor invisible
  start_color();            // Have some colors
  use_default_colors();
}

// Take an x coordinate and transate it for the screen
// This is due to the fact that thare a point on th escree is two character wide
// This is just for the representation
static int translate(const int x) { return x + x + 1; }

// color is one of the colors provided by ncurses
static void drawPointWithColor(const Screen *self, const Point pos,
                               const int color) {
  init_pair(color, color, -1);
  attrset(COLOR_PAIR(color)); // Set color

  mvprintw(pos.y + self->offset.y, translate(pos.x) + self->offset.x, "██");

  init_pair(7, COLOR_WHITE, -1);
  attrset(COLOR_PAIR(7)); // Restore white color
}

bool borders(const Screen *self, const Snake *snake) {
  return snake->head->x <= self->width && snake->head->x >= 0 &&
         snake->head->y <= self->height && snake->head->y >= 0;
}

void spawnOrb(Screen *self) {
  do {
    self->orb.x = rand() % self->width;
    self->orb.y = rand() % self->height;
  } while (self->grid[self->orb.x * self->width + self->orb.y]);

  drawPointWithColor(self, self->orb, COLOR_MAGENTA);
}

void updateScore(const Screen *self, const unsigned score) {
  mvprintw(self->offset.y - 2, self->offset.x - 1, "Score: %d", score);
}

void drawWalls(const Screen *self) {
  mvprintw(self->offset.y - 1, self->offset.x - 1, "╔");
  mvprintw(self->offset.y - 1, self->width * 2 + self->offset.x + 1, "╗");
  mvprintw(self->height + self->offset.y + 1, self->offset.x - 1, "╚");
  mvprintw(self->height + self->offset.y + 1,
           self->width * 2 + self->offset.x + 1, "╝");

  for (int i = self->offset.x; i <= self->width * 2 + self->offset.x; ++i) {
    mvprintw(self->offset.y - 1, i, "═");
    mvprintw(self->height + self->offset.y + 1, i, "═");
  }
  for (int j = self->offset.y; j <= self->height + self->offset.y; ++j) {
    mvprintw(j, self->offset.x - 1, "║");
    mvprintw(j, self->width * 2 + self->offset.x + 1, "║");
  }
}

void draw(const Screen *self, const Snake *snake, const bool growing,
          const Node *oldTail) {
  // Draw the new head added by Snake::advance()
  const Point headPos = {snake->head->x, snake->head->y};
  drawPointWithColor(self, headPos, COLOR_GREEN);
  // self->grid[snake->head->y * self->width + snake->head->x] = true;

  // Cover the old tail with a blank if the Snake has not grown
  if (!growing) {
    mvprintw(oldTail->y + self->offset.y,
             translate(oldTail->x) + self->offset.x, "  ");
    // self->grid[oldTail->y * self->width + oldTail->x] = false; // segfault
  }
}

bool gameOver(Screen *self, Snake *snake, const Point collision) {
  if (collision.x != -1 && collision.y != -1)
    drawPointWithColor(self, collision, COLOR_RED);

  // hide score count above the playing field
  mvhline(self->offset.y - 2, self->offset.x - 1, ' ', self->width);

  const char *fmt[] = {
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
      "┃                      Play again?                      ┃",
      "┃                          y/n                          ┃",
      "┃                                                       ┃",
      "┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛"};

  for (int i = self->offset.y + self->height / 2 - 7, j = 0;
       i < self->offset.y + self->height / 2 - 7 + 14; ++i, ++j)
    mvprintw(i, self->offset.x + self->width - 28, fmt[j], snake->length);

  while (true) {
    switch (getch()) {
    case 'y':
    case 'Y': {
      // Reset the game
      Screen *oldSelf = self;
      Snake *oldSnake = snake;
      self = newScreen();
      snake = newSnake((Point){self->width / 2, self->height / 2});
      destroyScreen(oldSelf);
      destroySnake(oldSnake);
      return true;
    }
    case 'n':
    case 'N':
    case 'q':
      return false;
    }
  }
}
