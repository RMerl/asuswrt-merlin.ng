include ../common.mak
include $(SRCBASE)/.config

OBJS = ifenslave.o
PROGS = ifenslave

CFLAGS += $(EXTRACFLAGS)
ifneq ($(HND_ROUTER),y)
CFLAGS += -Wall -Wstrict-prototypes -Os -I$(LINUXDIR)/include
endif


all: $(PROGS)


$(PROGS): ifenslave.c
	$(CC) $(CFLAGS) $^ -o $@

install: all
	$(STRIP) $(PROGS)
	install -D $(PROGS) $(INSTALLDIR)/bin/$(PROGS)

clean:
	rm -f *.o $(PROGS)
