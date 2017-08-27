AM_CPPFLAGS = \
	-I$(top_srcdir)/src/libstrongswan \
	-I$(top_srcdir)/src/libhydra \
	-I$(top_srcdir)/src/libcharon

AM_LDFLAGS = \
	-no-undefined

ipseclib_LTLIBRARIES = libsimaka.la

libsimaka_la_LIBADD = \
	$(top_builddir)/src/libstrongswan/libstrongswan.la

libsimaka_la_SOURCES = simaka_message.h simaka_message.c \
  simaka_crypto.h simaka_crypto.c simaka_manager.h simaka_manager.c \
  simaka_card.h simaka_provider.h simaka_hooks.h
