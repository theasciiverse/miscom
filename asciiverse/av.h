/* av.h: the Asciiverse build of miscom (see PATCHES.md). Included before
 * every source file of the game by asciiverse/Makefile (-include), so the
 * game's own files stay as they were.
 *
 * miscom draws an 80x24 screen at fixed positions on stdscr. Here it
 * draws on an 80x24 pad instead: the name stdscr means the pad (curses'
 * drawing macros use that name), printw() draws on it, refresh() copies
 * it to the middle of the terminal, and keys are read through curses
 * (av_getkey()), which also centres the screen again after a resize.
 *
 * Copyright (C) 2026 Asciiverse, under the GNU General Public License,
 * version 2 or later, as miscom. No warranty.
 */
#ifndef AV_H
#define AV_H

#include <stddef.h>
#include <curses.h>

extern WINDOW *av_win(void);
extern WINDOW *av_initscr(void);
extern int av_refresh(void);
extern int av_getkey(void);
extern int av_exedir(char *buf, size_t n);

#undef stdscr
#define stdscr av_win()
#undef printw
#define printw(...) wprintw(av_win(), __VA_ARGS__)
#undef refresh
#define refresh() av_refresh()
#define initscr() av_initscr()

#ifdef _WIN32
#define random() rand()
#define srandom(s) srand(s)
#endif

#endif
