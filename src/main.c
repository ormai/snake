#include <locale.h>
#include <ncurses.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "map.h"
#include "snake.h"

bool restartGame(Map *map, Snake *snake) { return false; }

int main(void) {
  setlocale(LC_ALL, ""); // Use the locale of the environment
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

  Map *map = newMap(getmaxx(stdscr) - 1, getmaxy(stdscr) - 1);
  Snake *snake =
      newSnake((Point){map->width / 2, map->height / 2}, rand() % WEST);

  bool gameOver = false;

  drawWalls(map);

  // GAME LOOP
  while (!gameOver || restartGame(map, snake)) {
    // fprintf(logfile, "Dir: %d, x: %d, y: %d\n", snake->direction,
    //         snake->head->x, snake->head->y);
    switch (getch()) { // Get keyboard input
    case 'w':
    case 'k':
    case KEY_UP:
      changeDirection(snake, NORTH);
      // fprintf(logfile, "Input received: move up\n");
      break;
    case 'l':
    case 'd':
    case KEY_RIGHT:
      changeDirection(snake, EAST);
      // fprintf(logfile, "Input received: move right\n");
      break;
    case 'j':
    case 's':
    case KEY_DOWN:
      changeDirection(snake, SOUTH);
      // fprintf(logfile, "Input received: move down\n");
      break;
    case 'h':
    case 'a':
    case KEY_LEFT:
      changeDirection(snake, WEST);
      // fprintf(logfile, "Input received: move left\n");
      break;
    case 'q':
      goto QUIT;
    }

    Node *oldTail = advance(snake);
    gameOver = collision(map, snake);
    // gameOver = selfCollision(snake);
    bool growing = false;


    if (snake->head->x == map->orb.x && snake->head->y == map->orb.y) {
      growing = true;
      grow(snake, oldTail); // reappend oldTail to the Snake
      spawnOrb(map);

      FILE *logfile = fopen("snake.log", "a");
      fprintf(logfile, "New Orb: (%d, %d)\n", map->orb.x, map->orb.y);
      fclose(logfile);
    } else
      destroyNode(oldTail);

    draw(map, snake, growing, oldTail);

    updateScore(map, snake->length);

    usleep(100000); // sleep 300ms
  }

QUIT:
  endwin();
  puts("You just died");

  return EXIT_SUCCESS;
}
