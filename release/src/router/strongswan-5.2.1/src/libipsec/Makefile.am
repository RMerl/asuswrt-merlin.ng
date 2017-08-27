ipseclib_LTLIBRARIES = libipsec.la

libipsec_la_SOURCES = \
ipsec.c ipsec.h \
esp_context.c esp_context.h \
esp_packet.c esp_packet.h \
ip_packet.c ip_packet.h \
ipsec_event_listener.h \
ipsec_event_relay.c ipsec_event_relay.h \
ipsec_policy.c ipsec_policy.h \
ipsec_policy_mgr.c ipsec_policy_mgr.h \
ipsec_processor.c ipsec_processor.h \
ipsec_sa.c ipsec_sa.h \
ipsec_sa_mgr.c ipsec_sa_mgr.h

libipsec_la_LIBADD = \
	$(top_builddir)/src/libstrongswan/libstrongswan.la

AM_CPPFLAGS = \
	-I$(top_srcdir)/src/libstrongswan

AM_LDFLAGS = \
	-no-undefined

EXTRA_DIST = Android.mk

# build optional plugins
########################

if MONOLITHIC
SUBDIRS =
else
SUBDIRS = .
endif
