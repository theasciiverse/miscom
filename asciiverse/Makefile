# Builds miscom for Asciiverse: make -f asciiverse/Makefile
#
#   CC, CFLAGS, LDFLAGS, CPPFLAGS as usual; CURSES_LIBS (default -lncurses)
#   SOUND: the sound options (default: sound on Linux, through /dev/dsp or
#          aplay; set it empty elsewhere)
#   EXE=.exe for Windows; BUILD, the folder for objects (default build)
#
# Every game source is compiled with asciiverse/av.h included first
# (-include), which centres the screen and reads keys through curses;
# asciiverse/av.c is compiled on its own.

CC ?= cc
CFLAGS ?= -O2
CURSES_LIBS ?= -lncurses
SOUND ?= -DSOUND_SUPPORT -DMORE_SOUNDBUF
EXE ?=
BUILD ?= build
SRCS = main.c draw.c wwn.c fire.c unix.c level.c sound.c
OBJS = $(SRCS:%.c=$(BUILD)/%.o) $(BUILD)/av.o

miscom$(EXE): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $(OBJS) $(CURSES_LIBS)

$(BUILD)/av.o: asciiverse/av.c
	@mkdir -p $(BUILD)
	$(CC) $(CFLAGS) $(CPPFLAGS) -MMD -MP -c -o $@ $<

$(BUILD)/%.o: %.c asciiverse/av.h
	@mkdir -p $(BUILD)
	$(CC) $(CFLAGS) $(CPPFLAGS) -DAV_BUILD $(SOUND) -include asciiverse/av.h \
		-MMD -MP -c -o $@ $<

-include $(wildcard $(BUILD)/*.d)

clean:
	rm -rf $(BUILD) miscom$(EXE)
