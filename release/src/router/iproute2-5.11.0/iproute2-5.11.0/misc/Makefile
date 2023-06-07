# SPDX-License-Identifier: GPL-2.0
SSOBJ=ss.o ssfilter_check.o ssfilter.tab.o
LNSTATOBJ=lnstat.o lnstat_util.o

TARGETS=ss nstat ifstat rtacct lnstat

include ../config.mk

ifeq ($(HAVE_BERKELEY_DB),y)
	TARGETS += arpd
endif

all: $(TARGETS)

ss: $(SSOBJ)
	$(QUIET_LINK)$(CC) $^ $(LDFLAGS) $(LDLIBS) -o $@

nstat: nstat.c
	$(QUIET_CC)$(CC) $(CFLAGS) $(CPPFLAGS) $(LDFLAGS) -o nstat nstat.c $(LDLIBS) -lm

ifstat: ifstat.c
	$(QUIET_CC)$(CC) $(CFLAGS) $(CPPFLAGS) $(LDFLAGS) -o ifstat ifstat.c $(LDLIBS) -lm

rtacct: rtacct.c
	$(QUIET_CC)$(CC) $(CFLAGS) $(CPPFLAGS) $(LDFLAGS) -o rtacct rtacct.c $(LDLIBS) -lm

arpd: arpd.c
	$(QUIET_CC)$(CC) $(CFLAGS) -I$(DBM_INCLUDE) $(CPPFLAGS) $(LDFLAGS) -o arpd arpd.c $(LDLIBS) -ldb

ssfilter.tab.c: ssfilter.y
	$(QUIET_YACC)$(YACC) -b ssfilter ssfilter.y

lnstat: $(LNSTATOBJ)
	$(QUIET_LINK)$(CC) $^ $(LDFLAGS) $(LDLIBS) -o $@

install: all
	install -m 0755 $(TARGETS) $(DESTDIR)$(SBINDIR)
	ln -sf lnstat $(DESTDIR)$(SBINDIR)/rtstat
	ln -sf lnstat $(DESTDIR)$(SBINDIR)/ctstat

clean:
	rm -f *.o $(TARGETS) ssfilter.c
