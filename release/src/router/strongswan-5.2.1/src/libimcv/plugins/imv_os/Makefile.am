AM_CPPFLAGS = \
	-I$(top_srcdir)/src/libstrongswan \
	-I$(top_srcdir)/src/libtncif \
	-I$(top_srcdir)/src/libimcv

AM_CFLAGS = \
	$(PLUGIN_CFLAGS)

imcv_LTLIBRARIES = imv-os.la

imv_os_la_LIBADD = $(top_builddir)/src/libimcv/libimcv.la \
	$(top_builddir)/src/libstrongswan/libstrongswan.la

imv_os_la_SOURCES = \
	imv_os.c imv_os_state.h imv_os_state.c \
	imv_os_agent.h imv_os_agent.c \
	imv_os_database.c imv_os_database.h

imv_os_la_LDFLAGS = -module -avoid-version -no-undefined

if !USE_WINDOWS
ipsec_PROGRAMS = pacman
pacman_SOURCES = pacman.c
pacman_LDADD = $(top_builddir)/src/libstrongswan/libstrongswan.la
pacman.o :	$(top_builddir)/config.status

EXTRA_DIST = pacman.sh
endif
