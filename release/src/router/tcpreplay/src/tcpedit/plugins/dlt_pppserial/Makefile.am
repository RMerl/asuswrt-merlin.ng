# $Id:$
# START OF: dlt_pppserial
# Note, if you add any files to your plugin, you will need to edit dlt_pppserial/Makefile.am
# add your .c files to libtcpedit_a_SOURCES
# add your .h files to noinst_HEADERS
# add any other files (like documentation, notes, etc) to EXTRA_DIST
# add your dependency information (see comment below)

libtcpedit_a_SOURCES += \
	%reldir%/pppserial.c

noinst_HEADERS += \
	%reldir%/pppserial.h \
	%reldir%/pppserial_api.h \
	%reldir%/pppserial_types.h

EXTRA_DIST += %reldir%/pppserial_opts.def

# dependencies for your plugin source code.  Edit as necessary
pppserial.c: \
	$(TCPEDIT_PLUGINS_DEPS) \
	%reldir%/pppserial.h \
	%reldir%/pppserial_api.h \
	%reldir%/pppserial_types.h

# Note:
# You probably don't want to touch anything below this line until the end of the plugin

DLT_STUB_DEPS += %reldir%/pppserial_opts.def

MOSTLYCLEANFILES += *~

# END OF: dlt_pppserial
