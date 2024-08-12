#include <stdlib.h>
#include <sys/ioctl.h>

#include "map.h"
#include "snake.h"
#include "term.h"
#include "window.h"

struct map *map_create(void) {
  struct map *map = malloc(sizeof(struct map));

  const struct winsize ws = get_term_size();

  map->width = ws.ws_col / 3; // see translate()
  map->height = ws.ws_row * 2 / 3;
  map->area = map->width * map->height;

  map->offset = (struct point){(ws.ws_col - map->width * 2) / 2,
                               (ws.ws_row - map->height) / 2};

  map->grid = malloc(sizeof(int * [map->height + 1]));
  for (int i = 0; i <= map->height; ++i) {
    map->grid[i] = calloc(map->width + 1, sizeof(int));
  }

  return map;
}

void map_destroy(struct map *map) {
  if (map != NULL) {
    if (map->grid != NULL) {
      for (int i = 0; i <= map->height; ++i) {
        free(map->grid[i]);
      }
      free(map->grid);
    }
    free(map);
    map = NULL;
  }
}

bool inside_walls(const struct map *map, const struct snake *snake) {
  const struct point head = snake->body[snake->length - 1];
  return head.x <= map->width && head.x >= 0 && head.y <= map->height &&
         head.y >= 0;
}

void spawn_apple(struct map *map) {
  // This is a critical point. With a big enough map and when the struct snake
  // is short there is no problem. But when progressing towards the completion
  // of the game the app will probably stall, trying to randomly get a correct
  // position for the orb. One solution I thought is creating a dynamic
  // structure that holds the set of current available struct points to choose
  // from to spawn a new orb. But this is a lot of code and could slow things
  // down anyway. So I will leave the problem open for now.
  do {
    map->apple.x = rand() % (map->width + 1);
    map->apple.y = rand() % (map->height + 1);
  } while (map->grid[map->apple.y][map->apple.x] == 1);

  set_color(MAGENTA);
  draw_point(map, map->apple);
}
