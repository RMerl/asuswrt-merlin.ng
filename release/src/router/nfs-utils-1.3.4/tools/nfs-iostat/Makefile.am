## Process this file with automake to produce Makefile.in
PYTHON_FILES =  nfs-iostat.py

man8_MANS	= nfsiostat.man

EXTRA_DIST	= $(man8_MANS) $(PYTHON_FILES)

all-local: $(PYTHON_FILES)

install-data-hook:
	$(INSTALL) -m 755 nfs-iostat.py $(DESTDIR)$(sbindir)/nfsiostat

MAINTAINERCLEANFILES=Makefile.in
