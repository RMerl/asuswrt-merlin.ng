if !USE_WINDOWS

sbin_PROGRAMS = sec-updater 

AM_CPPFLAGS = \
	-I$(top_srcdir)/src/libstrongswan \
	-DPLUGINS=\""sqlite"\"

sec_updater_SOURCES = sec-updater.c
sec_updater_LDADD = $(top_builddir)/src/libstrongswan/libstrongswan.la
sec-updater.o : $(top_builddir)/config.status

EXTRA_DIST = sec-updater.sh

man8_MANS = sec-updater.8
CLEANFILES = $(man8_MANS)

endif
