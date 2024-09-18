// SPDX-License-Identifier: GPL-3.0-only
// Copyright © 2024  Mario D'Andrea https://ormai.dev

#include <fcntl.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>

#include "term.h"

static struct termios saved_attr;

void term_init(void) {
  // Switch to alternative screen, so that the previous terminal can be restored
  printf(CSI "?1049h");

  tcgetattr(STDIN_FILENO, &saved_attr);
  nonblocking_input(true);

  struct termios t;
  tcgetattr(STDIN_FILENO, &t);
  t.c_lflag &= ~(ECHO | ICANON); // disable echo and canonical input mode
  tcsetattr(STDIN_FILENO, TCSANOW, &t);

  printf(CSI "?25l"); // make cursor invisible
}

void term_finalize(void) {
  printf(CSI "?25h"); // make cursor visible
  tcsetattr(STDIN_FILENO, TCSANOW, &saved_attr);
  printf(CSI "?1049l"); // switch back from alternative screen
}

int getch(void) {
  const int c = getchar();
  if (c == ESC) {
    getchar(); // skip [
    return ESC + '[' + getchar();
  }
  return c;
}

struct winsize get_term_size(void) {
  struct winsize w;
  ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
  return w;
}

void nonblocking_input(const bool enabled) {
  if (enabled) {
    fcntl(STDIN_FILENO, F_SETFL, O_NONBLOCK);
  } else {
    const int flag = fcntl(STDIN_FILENO, F_GETFL);
    fcntl(STDIN_FILENO, F_SETFL, flag & ~O_NONBLOCK);
  }
}

void set_color(const enum color color) { printf(CSI "%dm", color); }

void erase(void) { printf(CSI "2J"); }

static inline void move(const int y, const int x) {
  printf(CSI "%d;%dH", y + 1, x + 1);
}

void erase_line(const int y) {
  move(y, 0);
  printf(CSI "2K");
}

void print(const int y, const int x, const char *fmt, ...) {
  move(y, x);

  char buf[2048];
  va_list ap;
  va_start(ap, fmt);
  vsnprintf(buf, 2048, fmt, ap);
  buf[2047] = '\0';
  va_end(ap);

  printf("%s", buf);
}
