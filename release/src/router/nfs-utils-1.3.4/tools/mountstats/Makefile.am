## Process this file with automake to produce Makefile.in
PYTHON_FILES =  mountstats.py

man8_MANS	= mountstats.man

EXTRA_DIST	= $(man8_MANS) $(PYTHON_FILES)

all-local: $(PYTHON_FILES)

install-data-hook:
	$(INSTALL) -m 755 mountstats.py $(DESTDIR)$(sbindir)/mountstats

MAINTAINERCLEANFILES=Makefile.in
