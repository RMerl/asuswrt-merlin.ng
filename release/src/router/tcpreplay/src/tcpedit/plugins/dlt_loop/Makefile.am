# $Id:$
# START OF: dlt_loop
# Note, if you add any files to your plugin, you will need to edit dlt_<plugin>/Makefile.am
# add your .c files to libtcpedit_a_SOURCES
# add your .h files to noinst_HEADERS
# add any other files (like documentation, notes, etc) to EXTRA_DIST
# add your dependency information (see comment below)

libtcpedit_a_SOURCES += %reldir%/loop.c

noinst_HEADERS += %reldir%/loop.h

EXTRA_DIST += %reldir%/loop_opts.def

# dependencies for your plugin source code.  Edit as necessary
loop.c: \
	$(TCPEDIT_PLUGINS_DEPS) \
	%reldir%/loop.h

# You probably don't want to touch anything below this line until the end of the plugin

DLT_STUB_DEPS += %reldir%/loop_opts.def

MOSTLYCLEANFILES += *~

# END OF: dlt_loop
