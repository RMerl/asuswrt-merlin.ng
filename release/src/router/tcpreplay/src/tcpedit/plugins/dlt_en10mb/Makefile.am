# $Id:$
# START OF: dlt_en10mb
# Note, if you add any files to your plugin, you will need to edit dlt_<plugin>/Makefile.am
# add your .c files to libtcpedit_a_SOURCES
# add your .h files to noinst_HEADERS
# add any other files (like documentation, notes, etc) to EXTRA_DIST
# add your dependency information (see comment below)

libtcpedit_a_SOURCES += \
	%reldir%/en10mb.c \
	%reldir%/en10mb_api.c

noinst_HEADERS += \
	%reldir%/en10mb.h \
	%reldir%/en10mb_api.h \
	%reldir%/en10mb_types.h \
	%reldir%/../../tcpedit_types.h

EXTRA_DIST += %reldir%/en10mb_opts.def

# dependencies for your plugin source code.  Edit as necessary
en10mb.c: \
	$(TCPEDIT_PLUGINS_DEPS) \
	%reldir%/en10mb.h \
	%reldir%/en10mb_api.h \
	%reldir%/en10mb_types.h

# Note:
# You probably don't want to touch anything below this line until the end of the plugin

MOSTLYCLEANFILES += *~

# END OF: dlt_en10mb
