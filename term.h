// SPDX-License-Identifier: GPL-3.0-only
// Copyright © 2024  Mario D'Andrea https://ormai.me

// This header and the accompanying term.c provide a subset of the features of
// ncurses. This is accomplished by relying on the POSIX interface for the
// terminal, and on terminal escape sequences.
//
// - This helped me get started: https://github.com/byllgrim/ansicurses
// - Escape codes: https://gist.github.com/fnky/458719343aabd01cfb17a3a4f7296797

#ifndef TERM_H
#define TERM_H

#include <stdbool.h>
#include <sys/ioctl.h>

#define ESC '\033'
/// Control Sequence Introducer
#define CSI "\033[" 

enum color {
  BLACK = 30,
  RED,
  GREEN,
  YELLOW,
  BLUE,
  MAGENTA,
  CYAN,
  WHITE,
  DEFAULT_COLOR = 39,
  BRIGHT_BLACK = 90,
  BRIGHT_RED,
  BRIGHT_GREEN,
  BRIGHT_YELLO,
  BRIGHT_BLUE,
  BRIGHT_MAGENTA,
  BRIGHT_CYAN,
  BRIGHT_WHITE
};

enum arrow_key {
  ARROW_UP = '\033' + '[' + 'A',
  ARROW_DOWN,
  ARROW_RIGHT,
  ARROW_LEFT
};

/// Initializes the terminal, stores the current terminal state for later.
void term_init(void);

/// Restores the terminal behavior and its previous state.
void term_finalize(void);

/// Gets the current pressed key, if any.
/// 
/// Terminals treat the arrow keys as escape sequences up: `^[[A`, down: `^[[B`,
/// right: `^[[C`, left: `^[[D`; where `^[` is `ESC`.
/// This can be seen by launching `cat` without a file and pressing the arrow keys.
/// This function is just a wrapper around `getchar` to detect the arrow keys.
int getch(void);

/// Returns the terminal size in rows and cols inside the POSIX `struct winsize`.
struct winsize get_term_size(void);

/// Toggles non blocking mode for standard input.
void nonblocking_input(const bool enabled);

/// Sets the foreground color for standard output.
void set_color(const enum color color);

/// Erases everything on the terminal, like `clear` on the command line.
void erase(void);

/// Erases the `y`-th line terminal line.
void erase_line(const int y);

/// Prints a formatted string at line `y` and column `x`.
/// 
/// For multi byte characters to work an appropriate locale must be set.
void print(int y, int x, const char *fmt, ...);

#endif // TERM_H
