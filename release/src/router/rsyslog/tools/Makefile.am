sbin_PROGRAMS =
bin_PROGRAMS =
CLEANFILES =
man1_MANS =
man_MANS = rsyslogd.8 rsyslog.conf.5

sbin_PROGRAMS += rsyslogd
rsyslogd_SOURCES = \
	syslogd.c \
	rsyslogd.c \
	syslogd.h \
	omshell.c \
	omshell.h \
	omusrmsg.c \
	omusrmsg.h \
	omfwd.c \
	omfwd.h \
	omfile.c \
	omfile.h \
	ompipe.c \
	ompipe.h \
	omdiscard.c \
	omdiscard.h \
	pmrfc5424.c \
	pmrfc5424.h \
	pmrfc3164.c \
	pmrfc3164.h \
	smtradfile.c \
	smtradfile.h \
	smfile.c \
	smfile.h \
	smfwd.c \
	smfwd.h \
	smtradfwd.c \
	smtradfwd.h \
	iminternal.c \
	iminternal.h \
	\
	../dirty.h
rsyslogd_CPPFLAGS =  $(PTHREADS_CFLAGS) $(RSRT_CFLAGS) $(LIBLOGGING_STDLOG_CFLAGS) -DSD_EXPORT_SYMBOLS
# note: it looks like librsyslog.la must be explicitely given on LDDADD,
# otherwise dependencies are not properly calculated (resulting in a
# potentially incomplete build, a problem we had several times...)
rsyslogd_LDADD = ../grammar/libgrammar.la ../runtime/librsyslog.la ../compat/compat.la $(ZLIB_LIBS) $(PTHREADS_LIBS) $(RSRT_LIBS) $(SOL_LIBS) $(LIBUUID_LIBS) $(HASH_XXHASH_LIBS)

# Note: do NOT indent the if chain - it will not work!
if OS_LINUX
rsyslogd_LDFLAGS = -export-dynamic \
	#-Wl,--whole-archive,$(top_builddir)/runtime/.libs/librsyslog.a,--no-whole-archive
else
if OS_APPLE
rsyslogd_LDFLAGS = -export-dynamic \
	-Wl,$(top_builddir)/runtime/.libs/librsyslog.a
else
if OS_AIX
rsyslogd_LDFLAGS = -export-dynamic -bexpall -bE:strippedsymbols.exp
else # e.g. FreeBSD
rsyslogd_LDFLAGS = -export-dynamic
endif # if OS_AIX
endif # if OS_APPLE
endif # if OS_LINUX

EXTRA_DIST = $(man_MANS) \
	rscryutil.rst \
	recover_qi.pl

if ENABLE_LIBLOGGING_STDLOG
rsyslogd_LDADD += $(LIBLOGGING_STDLOG_LIBS)
endif

if ENABLE_DIAGTOOLS
sbin_PROGRAMS += rsyslog_diag_hostname msggen
rsyslog_diag_hostname_SOURCES = gethostn.c
msggen_SOURCES = msggen.c
endif

if ENABLE_USERTOOLS
if ENABLE_OMMONGODB
bin_PROGRAMS += logctl
logctl_SOURCES = logctl.c
logctl_CPPFLAGS = $(RSRT_CFLAGS) $(PTHREADS_CFLAGS) $(LIBMONGOC_CFLAGS)
logctl_LDADD = $(LIBMONGOC_LIBS)
endif
if ENABLE_LIBGCRYPT
bin_PROGRAMS += rscryutil
rscryutil = rscryutil.c
rscryutil_CPPFLAGS = -I../runtime $(RSRT_CFLAGS) $(LIBGCRYPT_CFLAGS)
rscryutil_LDADD = ../runtime/libgcry.la $(LIBGCRYPT_LIBS)
rscryutil_LDFLAGS = \
	-Wl,--whole-archive,--no-whole-archive
if ENABLE_GENERATE_MAN_PAGES
RSTMANFILE = rscryutil.rst
rscryutil.1: $(RSTMANFILE) 
	$(AM_V_GEN) $(RST2MAN) $(RSTMANFILE) $@
man1_MANS += rscryutil.1
CLEANFILES += rscryutil.1
EXTRA_DIST+= rscryutil.1
endif
endif
endif
