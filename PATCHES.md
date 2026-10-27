# Changes from upstream

This is the Asciiverse edition of miscom, Graham Richards' curses
missile defence game (1995 to 1996) as Russell Marks cleaned it up,
added colour and sound to, documented and uploaded (version 1.0, January
1998), GPL-2.0-or-later. It never had a repository; this one starts from
the release tarball on ibiblio (`pub/linux/games/arcade/miscom.tar.gz`,
sha256 `33778e8302cbb527e7040e953bcf5ee185b7a524455552e3e09ec1b532028e36`),
imported unchanged as the first commit (tag `1.0`). No distribution
packages it (not in Debian). The `asciiverse` branch carries the
changes below.

## Code

- **Builds with today's compilers.** unix.c used strlen() and exit()
  without their headers, and SIGINT was given bombout(), which takes no
  signal number (errors in GCC 14).
- **Ctrl-C quits outside the signal handler.** bombout() called curses
  from the handler; the handler now sets a flag, and getnbkey(), which
  every loop of the game polls, quits from there.
- **Pixels stay inside the field.** chkcoords() let y reach 40, one
  past the 40 half-rows of Screen[][]: a fireball at the ground wrote
  Screen[x][40] (in memory, a pixel at the top of the next column) and
  drew on the cities' row (found by UBSan).
- **Centred, keys through curses, and a resized terminal followed**
  (under `AV_BUILD`, with asciiverse/av.h included before every source
  by asciiverse/Makefile, so the game's files change only where they
  read keys): the 80x24 screen is drawn on a pad and shown in the middle
  of the terminal; keys come from curses instead of /dev/tty, so the
  arrow keys move the sight (as h j k l) and the game builds for
  Windows.
- **Sound without /dev/dsp.** Linux has had no OSS /dev/dsp for years,
  so the game was silent. When /dev/dsp cannot be opened, the sound
  process plays through ALSA's aplay (which also reaches PipeWire and
  PulseAudio), kept to the samples' own speed (8-bit, 8000 Hz); without
  aplay the game is silent as before. The samples are read from sounds/
  beside the program.
- **The sound process ends with the game.** It now closes its copy of
  the pipe's write end and stops when the pipe ends; before, if the game
  was killed or crashed, it stayed behind, holding the sound device,
  until the terminal was closed.
- `<linux/soundcard.h>` where there is no `<sys/soundcard.h>` (musl).

The level numbering stays as it is, from 0: the man page says it is
meant ("not a bug").

## Packaging

- `asciiverse/Makefile` builds with the changes above (sound on Linux;
  `SOUND=` without); `asciiverse/package.sh` packs `miscom-<target>`
  with the program, sounds/ (Linux), COPYING, README, ChangeLog, the man
  page and this file.
- `.github/workflows/release.yml`: static musl Linux with sound (Alpine,
  x86-64 and ARM64), macOS (Intel and Apple silicon, system ncurses) and
  Windows x86-64 (MinGW, PDCurses 3.9 wincon), both without sound, on
  our tags (`*-av.*`).
- `asciiverse/record.sh` records the catalog preview,
  `asciiverse/miscom.cast` (with a silent stand-in aplay first on PATH),
  while `asciiverse/drive.py` defends the cities: it reads the pane with
  its colours (incoming missiles are magenta), aims a little ahead of
  the warhead nearest the ground along its trail, and fires. The take
  kept is the best of several.

## Checked

Linux, native and static (Alpine): played at 100x30 and 120x40, arrow
keys, resizing; sound through aplay, the sound process and aplay ending
when the game is killed. AddressSanitizer and UBSan clean over
random-key and resize soaks after the fix above. Windows: cross-built
and run under Wine (a level played). macOS: not built here.

## Ours

- **How to play it, at the top of README.** A short note on installing and
  starting it with av, or from a release archive; the original text follows
  unchanged.
