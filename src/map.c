#include "map.h"
#include <stdlib.h>

Map *newMap(const unsigned width, const unsigned height) {
  Map *self = malloc(sizeof(Map));
  newOrb(self);
  self->grid = calloc(width * height, sizeof(bool));
  return self;
}

void destroyMap(Map *self) {
  if (self != NULL) {
    if (self->grid != NULL)
      free(self->grid);
    free(self);
  }
}

void newOrb(Map *self) {
  self->orb = (Point){rand() % self->width, rand() % self->height};

  // Exclude cells occupied by the snake;
}

bool collisions(const Map *self, const Snake *snake) {
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

void draw(const Map *self, const Snake *snake) {

}
