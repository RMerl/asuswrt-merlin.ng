# SPDX-License-Identifier: GPL-2.0
MAN7PAGES = $(wildcard *.7)

all:

distclean: clean

clean:

install:
	$(INSTALLDIR) $(DESTDIR)$(MANDIR)/man7
	$(INSTALLMAN) $(MAN7PAGES) $(DESTDIR)$(MANDIR)/man7

check:
	@for page in $(MAN7PAGES); do test 0 -eq $$($(MAN_CHECK) $$page \
		$(MAN_REDIRECT)) || { echo "Error in $$page"; exit 1; }; done

.PHONY: install clean distclean check
