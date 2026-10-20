/* av.c: the Asciiverse build of miscom: the centred screen, keys through
 * curses, and the program's own folder (see av.h and PATCHES.md).
 *
 * Copyright (C) 2026 Asciiverse, under the GNU General Public License,
 * version 2 or later, as miscom. No warranty.
 */
#include <stdio.h>
#include <string.h>
#include <curses.h>
#ifdef _WIN32
#include <windows.h>
#elif defined(__APPLE__)
#include <stdint.h>
#include <mach-o/dyld.h>
#else
#include <unistd.h>
#endif

#define AV_LINES 24
#define AV_COLS 80

static WINDOW *pad; /* where the game draws */

WINDOW *av_win(void) { return pad ? pad : stdscr; }

/* initscr(), then the pad. Keys are read on the terminal's own screen
 * (nothing draws there), without waiting. */
WINDOW *av_initscr(void) {
	WINDOW *w = initscr();

	pad = newpad(AV_LINES, AV_COLS);
	keypad(stdscr, TRUE);
	nodelay(stdscr, TRUE);
	/* nothing to show there: else the first key read would refresh it,
	 * blank, over the pad */
	untouchwin(stdscr);
	return w;
}

/* Show the pad centred (its top left part on a smaller terminal). */
int av_refresh(void) {
	int oy, ox, h, w;

	if (!pad) {
		return wrefresh(stdscr);
	}
	oy = LINES > AV_LINES ? (LINES - AV_LINES) / 2 : 0;
	ox = COLS > AV_COLS ? (COLS - AV_COLS) / 2 : 0;
	h = LINES < AV_LINES ? LINES : AV_LINES;
	w = COLS < AV_COLS ? COLS : AV_COLS;
	return prefresh(pad, 0, 0, oy, ox, oy + h - 1, ox + w - 1);
}

/* The next key, as the game's getnbkey() gave it (0 for none): the
 * arrow keys as h j k l, Return as 13. A resize centres the screen. */
int av_getkey(void) {
	int c;

	for (;;) {
		c = wgetch(stdscr);
#ifdef KEY_RESIZE
		if (c == KEY_RESIZE) {
#ifdef PDCURSES
			resize_term(0, 0);
#endif
			werase(stdscr);
			wnoutrefresh(stdscr);
			clearok(curscr, TRUE);
			av_refresh();
			continue;
		}
#endif
		switch (c) {
		case ERR: return 0;
		case KEY_UP: return 'k';
		case KEY_DOWN: return 'j';
		case KEY_LEFT: return 'h';
		case KEY_RIGHT: return 'l';
#ifdef KEY_ENTER
		case KEY_ENTER: return 13;
#endif
		}
		return c > 255 ? 0 : c;
	}
}

/* Copy the folder holding the running program into buf (of size n).
 * Return 0 if it cannot be found. */
int av_exedir(char *buf, size_t n) {
	char *p;
	long len;

#ifdef _WIN32
	len = GetModuleFileNameA(NULL, buf, (DWORD)n);
	if (len <= 0 || (size_t)len >= n) {
		return 0;
	}
	for (p = buf; *p; ++p) {
		if (*p == '\\') {
			*p = '/';
		}
	}
#elif defined(__APPLE__)
	uint32_t size = (uint32_t)n;

	if (_NSGetExecutablePath(buf, &size) != 0) {
		return 0;
	}
#else
	len = readlink("/proc/self/exe", buf, n - 1);
	if (len <= 0) {
		return 0;
	}
	buf[len] = '\0';
#endif
	if (!(p = strrchr(buf, '/'))) {
		return 0;
	}
	*p = '\0';
	return 1;
}
