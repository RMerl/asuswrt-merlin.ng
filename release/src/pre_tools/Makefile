
unexport AR AS LD CC CPP CXX NM STRIP SSTRIP OBJCOPY OBJDUMP RANLIB

CC = gcc
CFLAGS = -O3 -Wall

all: $(if ($(NCURSES_TOOLS)),ncurses)


ncurses:
ifeq ("$(NCURSES_TOOLS)","6.1")
	echo "build ncurses_6.1"
	cd ncurses-6.1 && ./configure && make
	cp ncurses-6.1/progs/tic $(SRCBASE)/router/ncurses-6.1/progs/tic
else
	echo "build other ncurses ver"
endif

clean:

%.o: %.c
	$(CC) $(CFLAGS) -c $<

.PHONY: ncurses
