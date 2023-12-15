# $Id:$
# START OF: dlt_linuxsll
# Note, if you add any files to your plugin, you will need to edit dlt_<plugin>/Makefile.am
# add your .c files to libtcpedit_a_SOURCES
# add your .h files to noinst_HEADERS
# add any other files (like documentation, notes, etc) to EXTRA_DIST
# add your dependency information (see comment below)

libtcpedit_a_SOURCES += %reldir%/linuxsll.c

noinst_HEADERS += \
	%reldir%/linuxsll.h \
	%reldir%/linuxsll_types.h

EXTRA_DIST += %reldir%/linuxsll_opts.def

# dependencies for your plugin source code.  Edit as necessary
linuxsll.c: \
	$(TCPEDIT_PLUGINS_DEPS) \
	%reldir%/../../tcpedit_api.h \
	%reldir%/linuxsll.h \
	%reldir%/linuxsll_types.h

# You probably don't want to touch anything below this line until the end of the plugin

DLT_STUB_DEPS += %reldir%/linuxsll_opts.def

MOSTLYCLEANFILES += *~

# END OF: dlt_linuxsll
