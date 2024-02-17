#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>

#include "map.h"

Map *newMap(const unsigned screenWidth, const unsigned screenHeight) {
  Map *self = malloc(sizeof(Map));

  self->screenWidth = screenWidth;
  self->screenHeight = screenHeight;
  self->width = screenWidth / 4;
  self->height = screenHeight * 2 / 3;

  self->offset = (Point){(screenWidth - self->width * 2) / 2,
                         (screenHeight - self->height) / 2};

  self->grid = calloc(self->width * self->height, sizeof(bool));
  spawnOrb(self);

  return self;
}

void destroyMap(Map *self) {
  if (self != NULL) {
    if (self->grid != NULL)
      free(self->grid);
    free(self);
  }
}

// Take an x coordinate and transate it for the screen
static int translate(int x) { return x + x + 1; }

bool collision(const Map *self, const Snake *snake) {
  if (snake->head->x > self->width || snake->head->x < 0 ||
      snake->head->y > self->height || snake->head->y < 0)
    return true;

  // Self-collision of the snake
  for (Node *it1 = snake->head; it1 != NULL; it1 = it1->next)
    for (Node *it2 = it1->next; it2 != NULL; it2 = it2->next)
      if (it1->x == it2->x && it1->y == it2->y)
        return true;

  return false;
}

void spawnOrb(Map *self) {
  do {
    self->orb.x = rand() % self->width;
    self->orb.y = rand() % self->height;
  } while (self->grid[self->orb.x * self->width + self->orb.y]);

  init_pair(1, COLOR_RED, -1);
  attrset(COLOR_PAIR(1));

  mvprintw(self->orb.y + self->offset.y,
           translate(self->orb.x) + self->offset.x, "██");

  init_pair(7, COLOR_WHITE, -1);
  attrset(COLOR_PAIR(7));
}

void updateScore(const Map *self, const unsigned score) {
  mvprintw(self->offset.y - 2, self->offset.x - 1, "Score: %d", score);
}

void drawWalls(const Map *self) {
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

void draw(const Map *self, const Snake *snake, const bool growing,
          const Node *oldTail) {
  // Draw the new head added by Snake::advance()
  mvprintw(snake->head->y + self->offset.y,
           translate(snake->head->x) + self->offset.x, "██");
  self->grid[snake->head->y * self->width + snake->head->x] = true;

  // Cover the old tail with a blank if the Snake has not grown
  if (!growing) {
    mvprintw(oldTail->y + self->offset.y,
             translate(oldTail->x) + self->offset.x, "  ");
    self->grid[oldTail->y * self->width + snake->head->x] = false;
  }
}
