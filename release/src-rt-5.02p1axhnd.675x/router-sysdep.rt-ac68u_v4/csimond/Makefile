PROGS     = csimond

INSTDIR   = $(INSTALLDIR)/usr/sbin/
INSTMODE  = 0755
INSTOWNER = root
INSTGROUP = root

OBJS      = csimond.o

CC        = $(CROSS_COMPILE)gcc
INSTALL   = install

all: $(PROGS)

CFLAGS   += -DSHELLPATH=\"$(run_prefix)/bin/sh\" -Wall -Os

$(PROGS): $(OBJS)
	$(CC) $(LDFLAGS) $^ $(LDLIBS) -o $@

.PHONY: install
install: $(PROGS)
	$(INSTALL) -d $(INSTDIR)
	$(INSTALL) -m $(INSTMODE) $(PROGS) $(INSTDIR)
	$(STRIP) $(foreach prog,$(PROGS),$(INSTDIR)/$(prog))

.PHONY: clean
clean:
	rm -f $(PROGS) *.o core
