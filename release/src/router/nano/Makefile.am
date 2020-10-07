AUTOMAKE_OPTIONS = gnu no-dependencies

SUBDIRS = doc lib m4 po src

if USE_COLOR
SUBDIRS += syntax
endif

EXTRA_DIST = IMPROVEMENTS

ACLOCAL_AMFLAGS = -I m4

showinfo:
	@ echo
	@ echo "  The global nanorc file is: @sysconfdir@/nanorc"
	@ echo "  Syntaxes get installed in: @PKGDATADIR@/"
	@ echo
