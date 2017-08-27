## Process this file with automake to produce Makefile.in
ACLOCAL_AMFLAGS = -I m4 ${ACLOCAL_FLAGS}

SUBDIRS = libsoup po tests docs

EXTRA_DIST =				\
	data/effective_tld_names.dat	\
	libsoup-2.4.pc.in		\
	libsoup-gnome-2.4.pc.in		\
	gtk-doc.make			\
	libsoup-zip.in			\
	m4/introspection.m4		\
	Makefile.glib

DISTCHECK_CONFIGURE_FLAGS = --enable-gtk-doc --enable-introspection

pkgconfigdir = $(libdir)/pkgconfig

pkgconfig_DATA = libsoup-2.4.pc

if BUILD_LIBSOUP_GNOME
pkgconfig_DATA += libsoup-gnome-2.4.pc
endif
