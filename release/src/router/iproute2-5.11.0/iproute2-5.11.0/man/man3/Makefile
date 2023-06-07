# SPDX-License-Identifier: GPL-2.0
MAN3PAGES = $(wildcard *.3)

all:

distclean: clean

clean:

install:
	$(INSTALLDIR) $(DESTDIR)$(MANDIR)/man3
	$(INSTALLMAN) $(MAN3PAGES) $(DESTDIR)$(MANDIR)/man3

check:
	@for page in $(MAN3PAGES); do test 0 -eq $$($(MAN_CHECK) $$page \
		$(MAN_REDIRECT)) || { echo "Error in $$page"; exit 1; }; done

.PHONY: install clean distclean check
