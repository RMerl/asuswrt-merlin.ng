lib_LTLIBRARIES        = libconfuse.la
include_HEADERS        = confuse.h
libconfuse_la_SOURCES  = confuse.c compat.h lexer.l
libconfuse_la_CPPFLAGS = -D_GNU_SOURCE -DBUILDING_DLL
libconfuse_la_LIBADD   = $(LTLIBINTL)
# -no-undefined is required for windows DLL support
libconfuse_la_LDFLAGS  = $(AM_LDFLAGS) -no-undefined -version-info 2:0:0

datadir                = @datadir@
localedir              = $(datadir)/locale
DEFS                   = -DLOCALEDIR=\"$(localedir)\" @DEFS@
LIBS                   = @LIBS@
AM_LFLAGS              = @DEFS@ -Pcfg_yy -olexer.c
EXTRA_DIST             = lexer.c
CLEANFILES             = *~ \#*\#

if MISSING_FMEMOPEN
libconfuse_la_SOURCES += fmemopen.c
endif
