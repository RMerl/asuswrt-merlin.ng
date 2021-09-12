CFLAGS ?= -O3 -march=native
CFLAGS += -std=gnu99
CFLAGS += -Wall
CFLAGS += -MMD -MP

highlight: highlight.o highlighter.o

fuzz: CC := clang
fuzz: CFLAGS += -fsanitize=fuzzer
fuzz: fuzz.c highlighter.c

gui/Makefile: gui/highlight.pro
	cd gui && qmake
gui: gui/Makefile
	@$(MAKE) -C gui

clean:
	rm -f highlight fuzz *.o *.d
	@if [ -f gui/Makefile ]; then $(MAKE) -C gui distclean; fi

.PHONY: clean gui
.DEFAULT_GOAL: highlight
MAKEFLAGS += --no-print-directory

-include *.d
