if INSTALL_M4
aclocaldir=$(datadir)/aclocal
aclocal_DATA=alsa.m4
endif
EXTRA_DIST=alsa.m4 buildrpm alsa.pc.in

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = alsa.pc

rpm: buildrpm alsa-lib.spec
	VERSION=$(VERSION) $(srcdir)/buildrpm

INCLUDES=-I$(top_srcdir)/include
