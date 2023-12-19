# $Id$
# START OF: dlt_radiotap
# Note, if you add any files to your plugin, you will need to edit dlt_radiotap/Makefile.am
# add your .c files to libtcpedit_a_SOURCES
# add your .h files to noinst_HEADERS
# add any other files (like documentation, notes, etc) to EXTRA_DIST
# add your dependency information (see comment below)

libtcpedit_a_SOURCES += %reldir%/radiotap.c

noinst_HEADERS += %reldir%/radiotap.h

EXTRA_DIST += %reldir%/radiotap_opts.def

# dependencies for your plugin source code.  Edit as necessary
radiotap.c: \
	$(TCPEDIT_PLUGINS_DEPS) \
	%reldir%/radiotap.h


# You probably don't want to touch anything below this line until the end of the plugin

DLT_STUB_DEPS += %reldir%/radiotap_opts.def

MOSTLYCLEANFILES += *~

# END OF: dlt_radiotap
