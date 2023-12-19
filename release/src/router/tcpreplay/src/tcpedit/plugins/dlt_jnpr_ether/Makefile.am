# $Id:$
# START OF: dlt_jnpr_ether
# Note, if you add any files to your plugin, you will need to edit dlt_jnpr_ether/Makefile.am
# add your .c files to libtcpedit_a_SOURCES
# add your .h files to noinst_HEADERS
# add any other files (like documentation, notes, etc) to EXTRA_DIST
# add your dependency information (see comment below)

libtcpedit_a_SOURCES += \
	%reldir%/jnpr_ether.c

noinst_HEADERS += \
	%reldir%/jnpr_ether.h \
	%reldir%/jnpr_ether_api.h \
	%reldir%/jnpr_ether_types.h

EXTRA_DIST += %reldir%/jnpr_ether_opts.def

# dependencies for your plugin source code.  Edit as necessary
jnpr_ether.c: \
	$(TCPEDIT_PLUGINS_DEPS) \
	%reldir%/jnpr_ether.h \
	%reldir%/jnpr_ether_api.h \
	%reldir%/jnpr_ether_types.h

# Note:
# You probably don't want to touch anything below this line until the end of the plugin

DLT_STUB_DEPS += %reldir%/jnpr_ether_opts.def

MOSTLYCLEANFILES += *~

# END OF: dlt_jnpr_ether
