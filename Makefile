CFLAGS = -Wall -Wextra
ifdef DEBUG
CFLAGS := $(CFLAGS) -ggdb
endif

.PHONY: all
all: ps2emu

gpio.o: gpio.h gpio.c
	$(CC) -c gpio.c $(CFLAGS)

ps2dev.o: ps2dev.h ps2dev.c
	$(CC) -c ps2dev.c $(CFLAGS)

ps2keymap.o: ps2keymap.h ps2keymap.c
	$(CC) -c ps2keymap.c $(CFLAGS)

ps2emu: gpio.o ps2dev.o ps2keymap.o
	$(CC) gpio.o ps2dev.o ps2keymap.o -o ps2emu $(CFLAGS)

.PHONY: clean
clean:
	$(RM) ps2emu gpio.o ps2dev.o ps2keymap.o
