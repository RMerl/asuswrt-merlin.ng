EXTRA_DIST      = simple.conf reread.conf ftp.conf test.conf nested.conf deprecated.conf
noinst_PROGRAMS = simple ftpconf cfgtest cli nested deprecated addsec parsebuf
AM_CPPFLAGS     = -I$(top_srcdir)/src
AM_LDFLAGS      = -L../src/
LIBS            = $(LTLIBINTL)
LDADD           = ../src/libconfuse.la
CLEANFILES      = *~ \#*\#

if !WINDOWS_BUILD
noinst_PROGRAMS += reread
else
noinst_PROGRAMS += wincfgtest
endif
