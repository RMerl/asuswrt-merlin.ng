AM_CFLAGS = -I $(top_srcdir)/include $(LLDP_CFLAGS)
AM_CPPFLAGS = $(LLDP_CPPFLAGS)
AM_LDFLAGS = $(LLDP_LDFLAGS)
BUILT_SOURCES =
CLEANFILES =

sbin_PROGRAMS = lldpd
man_MANS = lldpd.8

noinst_LTLIBRARIES = liblldpd.la

## Convenience library for lldpd and tests
nodist_liblldpd_la_SOURCES =
liblldpd_la_SOURCES  = \
	frame.h frame.c \
	lldp-tlv.h \
	client.c \
	priv.c \
	privsep.c privsep_io.c privsep_fd.c \
	interfaces.c \
	event.c lldpd.c \
	pattern.c \
	probes.d trace.h \
	protocols/lldp.c \
	protocols/cdp.c \
	protocols/cdp.h \
	protocols/sonmp.c \
	protocols/sonmp.h \
	protocols/edp.c \
	protocols/edp.h
liblldpd_la_CFLAGS   = $(AM_CFLAGS) @LIBEVENT_CFLAGS@
liblldpd_la_CPPFLAGS = $(AM_CPPFLAGS) -DSYSCONFDIR='"$(sysconfdir)"' -DLLDPCLI_PATH='"$(sbindir)/lldpcli"'
liblldpd_la_LIBADD   = \
	$(top_builddir)/src/libcommon-daemon-client.la \
	$(top_builddir)/src/libcommon-daemon-lib.la @LIBEVENT_LIBS@

## lldpd
lldpd_SOURCES = main.c
lldpd_LDFLAGS = $(AM_LDFLAGS) $(LLDP_BIN_LDFLAGS)
lldpd_LDADD   = liblldpd.la @LIBEVENT_LDFLAGS@

if HOST_OS_LINUX
liblldpd_la_SOURCES += \
	forward-linux.c \
	interfaces-linux.c \
	netlink.c \
	dmi-linux.c \
	priv-linux.c
endif
if HOST_OS_DRAGONFLY
liblldpd_la_SOURCES += \
	forward-bsd.c \
	interfaces-bpf.c \
	interfaces-bsd.c \
	dmi-dummy.c \
	priv-bsd.c
endif
if HOST_OS_FREEBSD
liblldpd_la_SOURCES += \
	forward-bsd.c \
	interfaces-bpf.c \
	interfaces-bsd.c \
	dmi-freebsd.c \
	priv-bsd.c
endif
if HOST_OS_OPENBSD
liblldpd_la_SOURCES += \
	interfaces-bpf.c \
	forward-bsd.c \
	interfaces-bsd.c \
	dmi-openbsd.c \
	priv-bsd.c
endif
if HOST_OS_NETBSD
liblldpd_la_SOURCES += \
	forward-bsd.c \
	interfaces-bpf.c \
	interfaces-bsd.c \
	dmi-dummy.c \
	priv-bsd.c
endif
if HOST_OS_OSX
liblldpd_la_SOURCES += \
	forward-bsd.c \
	interfaces-bpf.c \
	interfaces-bsd.c \
	dmi-osx.c \
	priv-bsd.c
liblldpd_la_LDFLAGS  = $(AM_LDFLAGS)
liblldpd_la_LDFLAGS += -framework Foundation
liblldpd_la_LDFLAGS += -framework CoreFoundation -framework IOKit
liblldpd_la_LDFLAGS += -framework IOKit
endif
if HOST_OS_SOLARIS
liblldpd_la_SOURCES += \
	forward-solaris.c \
	interfaces-bpf.c \
	interfaces-solaris.c \
	dmi-dummy.c \
	priv-bsd.c
endif

# Add SNMP support if needed
if USE_SNMP
liblldpd_la_SOURCES += agent.c agent_priv.c agent.h
liblldpd_la_CFLAGS  += @NETSNMP_CFLAGS@
lldpd_LDADD         += @NETSNMP_LIBS@
endif

# seccomp support
if USE_SECCOMP
BUILT_SOURCES += syscall-names.h
CLEANFILES    += syscall-names.h
syscall-names.h:
	$(AM_V_GEN)
	$(AM_V_at)echo "static const char *syscall_names[] = {" > $@ ;\
	echo "#include <sys/syscall.h>" | cpp -dM | grep '^#define __NR_' | \
		LC_ALL=C sed -r -n -e 's/^\#define[ \t]+__NR_([a-z0-9_]+)[ \t]+([0-9]+)(.*)/ [\2] = "\1",/p' >> $@ ;\
	echo "};" >> $@
nodist_liblldpd_la_SOURCES += syscall-names.h
liblldpd_la_SOURCES += priv-seccomp.c
liblldpd_la_CFLAGS  += @SECCOMP_CFLAGS@
liblldpd_la_LIBADD  += @SECCOMP_LIBS@
endif

## Systemtap/DTrace
EXTRA_DIST = dtrace2systemtap.awk
if ENABLE_SYSTEMTAP
BUILT_SOURCES += probes.h
CLEANFILES    += probes.h
probes.h: probes.d
	$(AM_V_GEN)
	$(AM_V_at)$(DTRACE) -C -h -s $< -o $@
probes.o: probes.d
	$(AM_V_GEN)
	$(AM_V_at)$(DTRACE) -C -G -s $< -o $@
lldpd_LDADD += probes.o

lldpd.stp: probes.d $(srcdir)/dtrace2systemtap.awk $(top_builddir)/config.status
	$(AM_V_GEN)$(AWK) -f $(srcdir)/dtrace2systemtap.awk -v sbindir=$(sbindir) $< > $@ || ( rm -f $@ ; exit 1 )
tapsetdir = $(datadir)/systemtap/tapset
tapset_DATA = lldpd.stp
endif

## libevent
if LIBEVENT_EMBEDDED
event.c: $(top_builddir)/libevent/libevent.la
$(top_builddir)/libevent/libevent.la: $(top_srcdir)/libevent/*.c $(top_srcdir)/libevent/*.h
	(cd $(top_builddir)/libevent && $(MAKE))
endif

## systemd service file
if HAVE_SYSTEMDSYSTEMUNITDIR
systemdsystemunit_DATA = lldpd.service
endif

if HAVE_SYSUSERSDIR
sysusers_DATA = lldpd.sysusers.conf
endif

if HOST_OS_LINUX
if HAVE_APPARMORDIR
apparmor_DATA = usr.sbin.lldpd
endif
endif

TEMPLATES   = lldpd.8    lldpd.service    lldpd.sysusers.conf    usr.sbin.lldpd
EXTRA_DIST += lldpd.8.in lldpd.service.in lldpd.sysusers.conf.in usr.sbin.lldpd.in
CLEANFILES += $(TEMPLATES)
lldpd.8: lldpd.8.in
lldpd.service: lldpd.service.in
lldpd.sysusers.conf: lldpd.sysusers.conf.in
usr.sbin.lldpd: usr.sbin.lldpd.in
include $(top_srcdir)/edit.am
