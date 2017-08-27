AM_CPPFLAGS = \
	-I$(top_srcdir)/src/libstrongswan

AM_CFLAGS = \
	$(PLUGIN_CFLAGS)

if MONOLITHIC
noinst_LTLIBRARIES = libstrongswan-agent.la
else
plugin_LTLIBRARIES = libstrongswan-agent.la
endif

libstrongswan_agent_la_SOURCES = \
	agent_plugin.h agent_plugin.c \
	agent_private_key.c agent_private_key.h

libstrongswan_agent_la_LDFLAGS = -module -avoid-version
