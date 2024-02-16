#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>

#include <ncurses.h>

#include "map.h"
#include "snake.h"

bool restartGame(Map *map, Snake *snake) {}

int main(int argc, char *argv[]) {
  // Initialize ncurses
  initscr();
  cbreak(); // Disable keybord input buffering, keys are immediately evaluated
  noecho(); // Disabl echoing for getch()
  intrflush(stdscr, false); // Flush the tty on quit
  keypad(stdscr, true);     // Enable keypad for the arrow keys
  nodelay(stdscr, true);    // getch() doesn't wait for input
  curs_set(0);              // Make the cursor invisible
  start_color();            // Have some colors
  use_default_colors();

  // Instantiate our objects
  Map *map = newMap(30, 30);
  Snake *snake =
      newSnake((Point){map->width / 2, map->height / 2}, rand() % WEST);

  bool gameOver = false;

  // GAME LOOP
  while (!gameOver || restartGame(map, snake)) {
    switch (getch()) { // Get keyboard input
    case 'W':
    case 'K':
    case KEY_UP:
      changeDirection(snake, NORTH);
      break;
    case 'L':
    case 'D':
    case KEY_RIGHT:
      changeDirection(snake, EAST);
      break;
    case 'J':
    case 'S':
    case KEY_DOWN:
      changeDirection(snake, SOUTH);
      break;
    case 'H':
    case 'A':
    case KEY_LEFT:
      changeDirection(snake, WEST);
      break;
    }

    advance(snake);
    gameOver = collision(map, snake);

    if (map->orb.x == snake->head->x && map->orb.x == snake->head->y) {
      grow(snake);
      newOrb(map);
    }

    draw(map, snake);

    usleep(10000);
  }

  endwin();

  return EXIT_SUCCESS;
}
