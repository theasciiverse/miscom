# makefile for miscom

# needs an ANSI C compiler - gcc is fine
CC=gcc

# change these if you want anything anywhere else
#
# bins go here:
BINDIR=/usr/games

# man page goes here:
MANDIR=/usr/man/man6

# sounds/* go here:
DATADIR=/usr/games/lib/miscom

# about `-DMORE_SOUNDBUF':
# With the new version of the sound driver in 1.3.x/2.0.x kernels,
# the sndfrag support seems very `clicky' and generally not as robust
# as it used to be, on my SB16 at least. So I decided to put up with
# slightly delayed sound, to get a better chance of decent output.
# If you don't think you need this, or you want more synchronous
# sound, uncomment the `-DMORE_SOUNDBUF' below.
#
# comment the line out if you don't want sound support, or if
# you're compiling on a non-Linux box.
SOUNDOPT=-DSOUNDSDIR=\"$(DATADIR)\" -DSOUND_SUPPORT -DMORE_SOUNDBUF

# compiler opts - shouldn't need changing. the `-I' is there just
# in case you're using an older ncurses.
CFLAGS=-O -I/usr/include/ncurses $(SOUNDOPT)

# how to link your curses lib. `-lncurses' on Linux (and newer BSD?),
# `-lcurses -ltermcap' on trad. BSD, `-lcurses -ltermlib' on SysV,
# etc.
LIBS=-lncurses

# you shouldn't need to change anything below this point.
#--------------------------------------------------------------------

FILES=main.o draw.o wwn.o fire.o unix.o level.o sound.o

miscom: $(FILES)
	$(CC) $(CFLAGS) -o miscom $(FILES) $(LIBS)

install:
	install -m 511 miscom $(BINDIR)
	install -m 444 miscom.6 $(MANDIR)
	-mkdir $(DATADIR)
	chmod 555 $(DATADIR)
	install -m 444 sounds/* $(DATADIR)

clean:
	$(RM) *.o *~ miscom
