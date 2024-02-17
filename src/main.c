#include <locale.h>
#include <ncurses.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>

#include "map.h"
#include "snake.h"

int main(void) {
  setlocale(LC_ALL, ""); // Use the locale of the environment
  initializeNcurses();

  // Instantiate the objects
  Map *map = newMap();
  Snake *snake = newSnake((Point){map->width / 2, map->height / 2});

  drawWalls(map);

  // GAME LOOP
  while (borders(map, snake) && !selfCollision(snake) || gameOver(map, snake)) {
    switch (getch()) { // Get keyboard input
    case 'w':
    case 'k':
    case KEY_UP:
      changeDirection(snake, NORTH);
      break;
    case 'l':
    case 'd':
    case KEY_RIGHT:
      changeDirection(snake, EAST);
      break;
    case 'j':
    case 's':
    case KEY_DOWN:
      changeDirection(snake, SOUTH);
      break;
    case 'h':
    case 'a':
    case KEY_LEFT:
      changeDirection(snake, WEST);
      break;
    case 'q':
      goto QUIT;
    }

    bool growing = false;
    Node *oldTail = advance(snake);
    if (snake->head->x == map->orb.x && snake->head->y == map->orb.y) {
      growing = true;
      grow(snake, oldTail); // reappend oldTail to the Snake
      spawnOrb(map);
    } else
      destroyNode(oldTail);

    draw(map, snake, growing, oldTail);

    updateScore(map, snake->length);

    usleep(100000); // sleep 300ms
  }

QUIT:
  destroySnake(snake);
  destroyMap(map);
  endwin();
  return EXIT_SUCCESS;
}
