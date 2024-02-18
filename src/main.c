#include <locale.h>
#include <ncurses.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>

#include "screen.h"
#include "snake.h"

int main(void) {
  setlocale(LC_ALL, ""); // Use the locale of the environment
  initializeNcurses();

  // Instantiate the objects
  Screen *screen = newScreen();
  Snake *snake = newSnake((Point){screen->width / 2, screen->height / 2});

  drawWalls(screen);

  Point collision = {-1, -1};

  // GAME LOOP
  while ((borders(screen, snake) && !selfCollision(snake, &collision)) ||
         gameOver(screen, snake, collision)) {
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
    if (snake->head->x == screen->orb.x && snake->head->y == screen->orb.y) {
      growing = true;
      grow(snake, oldTail); // reappend oldTail to the Snake
      spawnOrb(screen);
    } else
      destroyNode(oldTail);

    draw(screen, snake, growing, oldTail);

    updateScore(screen, snake->length);

    usleep(100000); // sleep 300ms
  }

QUIT:
  destroySnake(snake);
  destroyScreen(screen);
  endwin();
  return EXIT_SUCCESS;
}
