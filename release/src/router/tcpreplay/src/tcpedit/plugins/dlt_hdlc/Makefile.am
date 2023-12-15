# $Id:$
# START OF: dlt_hdlc
# Note, if you add any files to your plugin, you will need to edit dlt_<plugin>/Makefile.am
# add your .c files to libtcpedit_a_SOURCES
# add your .h files to noinst_HEADERS
# add any other files (like documentation, notes, etc) to EXTRA_DIST
# add your dependency information (see comment below)

libtcpedit_a_SOURCES += \
	%reldir%/hdlc.c \
	%reldir%/hdlc_api.c

noinst_HEADERS += \
	%reldir%/hdlc.h \
	%reldir%/hdlc_api.h \
	%reldir%/hdlc_types.h

EXTRA_DIST += %reldir%/hdlc_opts.def

# dependencies for your plugin source code.  Edit as necessary
hdlc.c: \
	$(TCPEDIT_PLUGINS_DEPS) \
	%reldir%/hdlc.h \
	%reldir%/hdlc_api.h \
	%reldir%/hdlc_types.h

# Note:
# You probably don't want to touch anything below this line until the end of the plugin

MOSTLYCLEANFILES += *~

# END OF: dlt_hdlc
