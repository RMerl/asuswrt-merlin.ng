# $Id: Makefile.am 1621 2006-11-09 07:01:37Z aturner $
noinst_LIBRARIES = libcommon.a

BUILT_SOURCES = git_version.c

git_version.c:
	$(ECHO) -n 'const char GIT_Version[] = "git:'   > git_version.c
	git describe --always | tr -d '\n' | tr -d '\r' >> git_version.c
	$(ECHO) '";'			>> git_version.c
	$(ECHO) 'const char *git_version(void) {'  >> git_version.c
	$(ECHO) '    return GIT_Version;'	>> git_version.c
	$(ECHO) '}' 			      	>> git_version.c

libcommon_a_SOURCES = cidr.c err.c list.c cache.c services.c get.c \
		      fakepcap.c fakepcapnav.c fakepoll.c xX.c utils.c \
		      timer.c git_version.c sendpacket.c \
		      dlt_names.c mac.c interface.c \
		      flows.c txring.c

if ENABLE_TCPDUMP
libcommon_a_SOURCES += tcpdump.c
endif

if COMPILE_NETMAP
libcommon_a_SOURCES += netmap.c
endif

AM_CFLAGS = -I$(srcdir)/.. -I$(srcdir)/../.. $(LNAV_CFLAGS) @LDNETINC@

if ! SYSTEM_STRLCPY
libcommon_a_LIBADD = ../../lib/libstrl.a
endif

noinst_HEADERS = cidr.h err.h list.h cache.h services.h get.h \
		 fakepcap.h fakepcapnav.h fakepoll.h xX.h utils.h \
		 tcpdump.h timer.h pcap_dlt.h sendpacket.h \
		 dlt_names.h mac.h interface.h flows.h txring.h \
		 netmap.h

MOSTLYCLEANFILES = *~

MAINTAINERCLEANFILES = Makefile.in git_version.c
