SSOBJ=ss.o ssfilter.o
LNSTATOBJ=lnstat.o lnstat_util.o

TARGETS=ss nstat ifstat rtacct lnstat

include ../Config

ifeq ($(HAVE_BERKELEY_DB),y)
	TARGETS += arpd
endif

ifeq ($(HAVE_SELINUX),y)
	LDLIBS += $(shell pkg-config --libs libselinux)
	CFLAGS += $(shell pkg-config --cflags libselinux) -DHAVE_SELINUX
endif

ifeq ($(IP_CONFIG_SETNS),y)
	CFLAGS += -DHAVE_SETNS
endif

all: $(TARGETS)

ss: $(SSOBJ)

nstat: nstat.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o nstat nstat.c $(LIBNETLINK) -lm

ifstat: ifstat.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o ifstat ifstat.c $(LIBNETLINK) -lm

rtacct: rtacct.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o rtacct rtacct.c $(LIBNETLINK) -lm

arpd: arpd.c
	$(CC) $(CFLAGS) -I$(DBM_INCLUDE) $(LDFLAGS) -o arpd arpd.c $(LIBNETLINK) -ldb -lpthread

ssfilter.c: ssfilter.y
	bison ssfilter.y -o ssfilter.c

lnstat: $(LNSTATOBJ)

install: all
	install -m 0755 $(TARGETS) $(DESTDIR)$(SBINDIR)
	ln -sf lnstat $(DESTDIR)$(SBINDIR)/rtstat
	ln -sf lnstat $(DESTDIR)$(SBINDIR)/ctstat

clean:
	rm -f *.o $(TARGETS) ssfilter.c
