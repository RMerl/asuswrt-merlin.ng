SUBDIRS=dbus bus tools test doc

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = dbus-1.pc

DISTCLEANFILES = 		\
	dbus-1.pc

EXTRA_DIST =			\
	autogen.sh		\
	HACKING			\
	dbus-1.pc.in		\
	cleanup-man-pages.sh    \
        ChangeLog.pre-1-0       \
        NEWS.pre-1-0            \
	ChangeLog.pre-1-2       \
        NEWS.pre-1-2		\
	README.valgrind		\
	README.win		\
	README.wince		\
	README.cygwin		\
	README.launchd		\
	cmake

all-local: Doxyfile

update-authors:
	git shortlog -s -e | cut -c 8- | sort > AUTHORS

DISTCHECK_CONFIGURE_FLAGS = \
	--enable-xml-docs \
	--with-systemdsystemunitdir=$$dc_install_base/$(systemdsystemunitdir)

ACLOCAL_AMFLAGS = -I m4 ${ACLOCAL_FLAGS}

include tools/lcov.am
