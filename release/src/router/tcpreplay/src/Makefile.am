# $Id: Makefile.am 1632 2007-02-03 18:46:16Z aturner $
if COMPILE_FRAGROUTE
    LIBFRAGROUTE = ./fragroute/libfragroute.a @LDNETLIB@
    FRAGROUTE_DIR = fragroute
else
    LIBFRAGROUTE = 
    FRAGROUTE_DIR = 
endif

SUBDIRS = common tcpedit $(FRAGROUTE_DIR)

if SYSTEM_STRLCPY
LIBSTRL =
else
LIBSTRL = ../lib/libstrl.a
endif

BUILT_SOURCES = 

manpages: tcpprep.1 tcprewrite.1 tcpreplay.1 tcpbridge.1 tcpreplay-edit.1 tcpliveplay.1 tcpcapinfo.1

autoopts: tcpreplay_opts.c tcprewrite_opts.c tcpbridge_opts.c tcpliveplay_opts.c tcpcapinfo_opts.c \
	manpages tcpreplay_edit_opts.c


# Get AutoOpts search path
opts_list=-L tcpedit

tcpprep.1: tcpprep_opts.def
	@AUTOGEN@ -T agman-cmd.tpl $(opts_list) $<

tcprewrite.1: tcprewrite_opts.def tcpedit/tcpedit_opts.def
	@AUTOGEN@ -T agman-cmd.tpl $(opts_list) $<

tcpreplay-edit.1: tcpreplay_opts.def
	@AUTOGEN@ -T agman-cmd.tpl $(opts_list) -DTCPREPLAY_EDIT -DTCPREPLAY_EDIT_MAN $<

tcpreplay.1: tcpreplay_opts.def
	@AUTOGEN@ -T agman-cmd.tpl $(opts_list) $<

tcpbridge.1: tcpbridge_opts.def
	@AUTOGEN@ -T agman-cmd.tpl $(opts_list) $<

tcpliveplay.1: tcpliveplay_opts.def
	@AUTOGEN@ -T agman-cmd.tpl $(opts_list) $<

tcpcapinfo.1: tcpcapinfo_opts.def
	@AUTOGEN@ -T agman-cmd.tpl $(opts_list) $<

man_MANS = tcpreplay.1 tcpprep.1 tcprewrite.1 tcpreplay-edit.1 tcpcapinfo.1
EXTRA_DIST = tcpreplay.1 tcpprep.1 tcprewrite.1 tcpbridge.1 tcpreplay-edit.1 \
	tcpliveplay.1 tcpcapinfo.1
bin_PROGRAMS = tcpreplay tcpprep tcprewrite tcpreplay-edit tcpcapinfo

if COMPILE_TCPBRIDGE
bin_PROGRAMS += tcpbridge 
man_MANS += tcpbridge.1
endif

if COMPILE_TCPLIVEPLAY
bin_PROGRAMS += tcpliveplay
man_MANS += tcpliveplay.1
endif

tcpreplay_edit_CFLAGS = $(LIBOPTS_CFLAGS) -I$(srcdir)/.. -I$(srcdir)/tcpedit $(LNAV_CFLAGS) @LDNETINC@ -DTCPREPLAY -DTCPREPLAY_EDIT -DHAVE_CACHEFILE_SUPPORT
tcpreplay_edit_LDADD = ./tcpedit/libtcpedit.a ./common/libcommon.a $(LIBSTRL) @LPCAPLIB@ @LDNETLIB@ $(LIBOPTS_LDADD)
tcpreplay_edit_SOURCES = tcpreplay_edit_opts.c send_packets.c signal_handler.c tcpreplay.c tcpreplay_api.c sleep.c replay.c
tcpreplay_edit_OBJECTS: tcpreplay_opts.h
tcpreplay_edit_opts.h: tcpreplay_edit_opts.c

BUILT_SOURCES += tcpreplay_edit_opts.h
tcpreplay_edit_opts.c: tcpreplay_opts.def
	@AUTOGEN@ $(opts_list)  @NETMAPFLAGS@ -DTCPREPLAY_EDIT -b tcpreplay_edit_opts $<

tcpreplay_CFLAGS = $(LIBOPTS_CFLAGS) -I$(srcdir)/.. $(LNAV_CFLAGS) @LDNETINC@ -DTCPREPLAY
tcpreplay_SOURCES = tcpreplay_opts.c send_packets.c signal_handler.c tcpreplay.c tcpreplay_api.c sleep.c replay.c
tcpreplay_LDADD = ./common/libcommon.a $(LIBSTRL) @LPCAPLIB@ @LDNETLIB@ $(LIBOPTS_LDADD)
tcpreplay_OBJECTS: tcpreplay_opts.h
tcpreplay_opts.h: tcpreplay_opts.c

BUILT_SOURCES += tcpreplay_opts.h
tcpreplay_opts.c: tcpreplay_opts.def
	@AUTOGEN@ $(opts_list) @NETMAPFLAGS@ $<

if ENABLE_OSX_FRAMEWORKS
tcpreplay_LDFLAGS = -framework CoreServices -framework Carbon
tcpreplay_edit_LDFLAGS = -framework CoreServices -framework Carbon
endif

tcpliveplay_CFLAGS = $(LIBOPTS_CFLAGS) -I$(srcdir)/.. $(LNAV_CFLAGS) -DTCPREPLAY -DTCPLIVEPLAY
tcpliveplay_SOURCES = tcpliveplay_opts.c tcpliveplay.c
tcpliveplay_LDADD = ./common/libcommon.a $(LIBSTRL) @LPCAPLIB@ @LDNETLIB@ $(LIBOPTS_LDADD)
tcpliveplay_OBJECTS: tcpliveplay_opts.h
tcpliveplay_opts.h: tcpliveplay_opts.c

BUILT_SOURCES += tcpliveplay_opts.h
tcpliveplay_opts.c: tcpliveplay_opts.def
	@AUTOGEN@ $(opts_list) $<


tcprewrite_CFLAGS = $(LIBOPTS_CFLAGS) -I$(srcdir)/.. -I$(srcdir)/tcpedit @LDNETINC@ $(LNAV_CFLAGS) -DTCPREWRITE -DHAVE_CACHEFILE_SUPPORT
tcprewrite_LDADD = ./tcpedit/libtcpedit.a ./common/libcommon.a \
	$(LIBSTRL) @LPCAPLIB@ $(LIBOPTS_LDADD) @DMALLOC_LIB@ \
	$(LIBFRAGROUTE)
tcprewrite_SOURCES = tcprewrite_opts.c tcprewrite.c 
tcprewrite_OBJECTS: tcprewrite_opts.h
tcprewrite_opts.h: tcprewrite_opts.c

BUILT_SOURCES += tcprewrite_opts.h
tcprewrite_opts.c: tcprewrite_opts.def tcpedit/tcpedit_opts.def
	@AUTOGEN@ $(opts_list) $<

tcpcapinfo_CFLAGS = $(LIBOPTS_CFLAGS) -I$(srcdir)/.. $(LNAV_CFLAGS) @LDNETINC@ -DTCPCAPINFO
tcpcapinfo_LDADD = ./common/libcommon.a \
    $(LIBSTRL) @LPCAPLIB@ $(LIBOPTS_LDADD) @DMALLOC_LIB@
tcpcapinfo_SOURCES = tcpcapinfo_opts.c tcpcapinfo.c
tcpcapinfo_OBJECTS: tcpcapinfo_opts.h
tcpcapinfo_opts.h: tcpcapinfo_opts.c

BUILT_SOURCES += tcpcapinfo_opts.h
tcpcapinfo_opts.c: tcpcapinfo_opts.def
	@AUTOGEN@ $(opts_list) $<

tcpprep_CFLAGS = $(LIBOPTS_CFLAGS) -I$(srcdir)/.. $(LNAV_CFLAGS) @LDNETINC@ -DTCPPREP
tcpprep_LDADD = ./common/libcommon.a \
    $(LIBSTRL) @LPCAPLIB@ $(LIBOPTS_LDADD) @DMALLOC_LIB@
tcpprep_SOURCES = tcpprep_opts.c tcpprep.c tree.c tcpprep_api.c
tcpprep_OBJECTS: tcpprep_opts.h
tcpprep_opts.h: tcpprep_opts.c

BUILT_SOURCES += tcpprep_opts.h
tcpprep_opts.c: tcpprep_opts.def
	@AUTOGEN@ $<

tcpbridge_CFLAGS = $(LIBOPTS_CFLAGS) -I$(srcdir)/.. -I$(srcdir)/tcpedit $(LNAV_CFLAGS) @LDNETINC@ -DTCPBRIDGE
tcpbridge_LDADD = ./tcpedit/libtcpedit.a ./common/libcommon.a \
    $(LIBSTRL) @LPCAPLIB@ @LDNETLIB@ $(LIBOPTS_LDADD) @DMALLOC_LIB@
if ENABLE_OSX_FRAMEWORKS
tcpbridge_LDFLAGS = -framework CoreServices -framework Carbon
endif
tcpbridge_SOURCES = tcpbridge_opts.c tcpbridge.c bridge.c
tcpbridge_OBJECTS: tcpbridge_opts.h
tcpbridge_opts.h: tcpbridge_opts.c

BUILT_SOURCES += tcpbridge_opts.h
tcpbridge_opts.c: tcpbridge_opts.def tcpedit/tcpedit_opts.def
	@AUTOGEN@ $(opts_list) $<

noinst_HEADERS = tcpreplay.h tcpprep.h bridge.h defines.h tree.h tcpliveplay.h \
		 send_packets.h signal_handler.h common.h tcpreplay_opts.h tcpliveplay_opts.h \
		 tcpreplay_edit_opts.h tcprewrite.h tcprewrite_opts.h tcpprep_opts.h \
		 tcpprep_opts.def tcprewrite_opts.def tcpreplay_opts.def tcpliveplay_opts.def \
		 tcpbridge_opts.def tcpbridge.h tcpbridge_opts.h tcpr.h sleep.h tcpcapinfo_opts.h \
		 tcpcapinfo_opts.def replay.h tcpreplay_api.h tcpprep_api.h timestamp_trace.h \
		 msvc_inttypes.h msvc_stdint.h

MOSTLYCLEANFILES = *~ *.o

MAINTAINERCLEANFILES = Makefile.in tcpreplay_opts.h tcpreplay_opts.c tcpliveplay_opts.c \
		       tcprewrite_opts.c tcprewrite_opts.h tcpprep_opts.c tcpliveplay_opts.h tcpcapinfo_opts.h \
		       tcpcapinfo_opts.c tcpprep_opts.h tcpprep.1 tcpreplay.1 tcprewrite.1 tcpliveplay.1 tcpcapinfo.1 \
		       tcpbridge.1 tcpbridge_opts.h tcpbridge_opts.c \
			   tcpreplay_edit_opts.c tcpreplay_edit_opts.h \
			   tcpreplay-edit.1
