# $Id$
# START OF: dlt_ieee80211
# Note, if you add any files to your plugin, you will need to edit dlt_ieee80211/Makefile.am
# add your .c files to libtcpedit_a_SOURCES
# add your .h files to noinst_HEADERS
# add any other files (like documentation, notes, etc) to EXTRA_DIST
# add your dependency information (see comment below)

libtcpedit_a_SOURCES += \
	%reldir%/ieee80211.c \
	%reldir%/ieee80211_hdr.c

noinst_HEADERS += \
	%reldir%/ieee80211.h \
	%reldir%/ieee80211_hdr.h \
	%reldir%/ieee80211_types.h

EXTRA_DIST += %reldir%/ieee80211_opts.def

# dependencies for your plugin source code.  Edit as necessary
ieee80211.c: \
	$(TCPEDIT_PLUGINS_DEPS) \
	%reldir%/ieee80211_hdr.c \
	%reldir%/ieee80211_hdr.h \
	%reldir%/ieee80211_types.h \
	%reldir%/ieee80211.h

# You probably don't want to touch anything below this line until the end of the plugin

DLT_STUB_DEPS += %reldir%/ieee80211_opts.def

MOSTLYCLEANFILES += *~

# END OF: dlt_ieee80211
