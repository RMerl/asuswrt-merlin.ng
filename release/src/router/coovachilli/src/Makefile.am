AUTOMAKE_OPTIONS=foreign

SUBDIRS = 
EXTRA_DIST = cmdline.ggo cmdline.patch \
linux/Makefile linux/xt_coova.c linux/xt_coova.h \
linux/ipt_coova.h linux/libxt_coova.c \
cmdline.mini.sh cmdline.mini.gen 

chilliincludedir = $(includedir)/chilli
chilliinclude_HEADERS = \
chilli.h session.h garden.h radius.h options.h \
tun.h ippool.h md5.h redir.h dhcp.h syserr.h iphash.h \
radius_wispr.h radius_chillispot.h ssl.h dns.h net.h \
pkt.h conn.h lookup.h chilli_limits.h cmdline.h debug.h \
radius_pkt.h ../bstring/bstrlib.h ../config.h system.h 

lib_LTLIBRARIES = libchilli.la
sbin_PROGRAMS = \
chilli chilli_response chilli_radconfig chilli_opt 

suid_programs =

libchilli_la_SOURCES = \
chilli.c tun.c ippool.c radius.c md5.c redir.c dhcp.c syserr.c \
iphash.c lookup.c system.h util.c options.c statusfile.c conn.c sig.c \
garden.c dns.c session.c pkt.c chksum.c net.c safe.c

AM_CFLAGS = -D_GNU_SOURCE -Wall -Werror -fno-builtin -fno-strict-aliasing \
  -O2 -fomit-frame-pointer -funroll-loops -pipe -I$(top_builddir)/bstring \
 -DDEFCHILLICONF='"$(sysconfdir)/chilli.conf"'\
 -DDEFPIDFILE='"$(localstatedir)/run/chilli.pid"'\
 -DDEFSTATEDIR='"$(localstatedir)/run"'\
 -DSBINDIR='"$(sbindir)"' 

chilli_SOURCES= main.c
chilli_radconfig_SOURCES = main-radconfig.c 
chilli_response_SOURCES = main-response.c 
chilli_query_SOURCES = main-query.c 
chilli_opt_SOURCES = main-opt.c 

libchilli_la_LIBADD = $(top_builddir)/bstring/libbstring.la 
LDADD = libchilli.la ${LIBRT} $(top_builddir)/bstring/libbstring.la 

if WITH_MINICONFIG
chilli_opt_SOURCES += cmdline.mini.c
else
chilli_opt_SOURCES += cmdline.c 
endif

if WITH_PATRICIA
libchilli_la_SOURCES += patricia.c patricia.h
endif

if WITH_JSONLIB
AM_CFLAGS += -I$(top_builddir)/json
libchilli_la_LIBADD += $(top_builddir)/json/libjson.la 
LDADD += $(top_builddir)/json/libjson.la 
endif

if WITH_EWTAPI
libchilli_la_SOURCES += ewt.c ewt.h
AM_CFLAGS += -std=gnu99 
endif

if WITH_LOCATION
libchilli_la_SOURCES += location.c 
endif

if WITH_SFHASH
libchilli_la_SOURCES += sfhash.c
endif

if WITH_LOOKUP3
libchilli_la_SOURCES += lookup3.c
endif

if WITH_OPENSSL
libchilli_la_LIBADD += ${LIBSSL}
LDADD += ${LIBSSL}
endif

if WITH_CYASSL
libchilli_la_LIBADD += ${LIBSSL}
LDADD += ${LIBSSL}
endif

if WITH_MATRIXSSL
libchilli_la_SOURCES += mssl.c mssl.h
libchilli_la_LIBADD += -lmatrixssl
LDADD += -lmatrixssl
if WITH_MATRIXSSL_CLI
SUBDIRS = mssl
endif
endif

if WITH_AVL
libchilli_la_SOURCES += avl/avl.c avl/avl.h avl/list.h \
avl/common_types.h avl/container_of.h
endif

if WITH_SSL
libchilli_la_SOURCES += ssl.c ms_chap.c 
endif

if WITH_NETFILTER_QUEUE
libchilli_la_SOURCES += queue.c
LDADD += ${LIBNETFILTER_QUEUE}
endif

if WITH_NETNAT
libchilli_la_SOURCES += nat.c nat.h
endif

if WITH_PCAP
LDADD += ${LIBPCAP}
endif

if WITH_MULTIROUTE
sbin_PROGRAMS += chilli_rtmon
chilli_rtmon_SOURCES = main-rtmon.c 
libchilli_la_SOURCES += rtmon.c rtmon.h
endif

if WITH_CHILLIQUERY
sbin_PROGRAMS += chilli_query
libchilli_la_SOURCES += cmdsock.c
chilliinclude_HEADERS += cmdsock.h
endif

if WITH_CHILLIPROXY
sbin_PROGRAMS += chilli_proxy
chilli_proxy_SOURCES = main-proxy.c 
chilli_proxy_LDADD = ${LDADD}
if WITH_CURL
chilli_proxy_LDADD += -lcurl -lz 
endif
endif

if WITH_CHILLIRADSEC
sbin_PROGRAMS += chilli_radsec
chilli_radsec_SOURCES = main-radsec.c 
chilli_radsec_LDADD = ${LDADD}
endif

if WITH_CHILLISCRIPT
sbin_PROGRAMS += chilli_script
suid_programs += chilli_script
chilli_script_SOURCES = main-script.c 
chilli_script_LDADD = ${LDADD}
endif

if WITH_CHILLIREDIR
sbin_PROGRAMS += chilli_redir
chilli_redir_SOURCES = main-redir.c 
chilli_redir_LDADD = ${LDADD}
if WITH_CURL
chilli_redir_LDADD += -lcurl -lz 
endif
endif

if WITH_NETFILTER_COOVA
SUBDIRS += linux
libchilli_la_SOURCES += kcoova.c
endif

if WITH_MODULES
sample_la_SOURCES = sample-mod.c
sample_la_LDFLAGS = -module -avoid-version
pkglibdir = $(libdir)/coova-chilli
pkglib_LTLIBRARIES = sample.la
libchilli_la_SOURCES += chilli_module.c
libchilli_la_LIBADD += -ldl
LDADD += -ldl
chilliinclude_HEADERS += chilli_module.h
AM_CFLAGS += -DDEFLIBDIR='"$(libdir)/coova-chilli"'
endif

CMDLINE = cmdline.ggo
if WITH_CONFIG
CMDLINE += `[ -e ../../cmdline.ggo ] && echo ../../cmdline.ggo`
endif

install-exec-hook:
	@for f in $(suid_programs); do \
		echo "Setting SUID on $(DESTDIR)$(sbindir)/$$f !"; \
		chown root $(DESTDIR)$(sbindir)/$$f; \
		chmod u+s $(DESTDIR)$(sbindir)/$$f; \
	done

main-opt.c: cmdline.c cmdline.h

cmdline.c cmdline.h: cmdline.ggo cmdline.patch
	cat $(CMDLINE) | gengetopt -C 
	cp cmdline.c cmdline.c.orig
	patch -p0 < cmdline.patch

cmdline.mini.gen: cmdline.ggo 
	sh cmdline.mini.sh > cmdline.mini.gen

cmdline.mini.c: cmdline.mini.gen
