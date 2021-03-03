# SPDX-License-Identifier: GPL-2.0
TARGETS = ip-address.8 ip-link.8 ip-netns.8 ip-route.8

MAN8PAGES = $(TARGETS) $(filter-out $(TARGETS),$(wildcard *.8))

all: $(TARGETS)

%: %.in
	sed \
		-e "s|@NETNS_ETC_DIR@|$(NETNS_ETC_DIR)|g" \
		-e "s|@NETNS_RUN_DIR@|$(NETNS_RUN_DIR)|g" \
		-e "s|@SYSCONFDIR@|$(CONFDIR)|g" \
		$< > $@

distclean: clean

clean:
	@rm -f $(TARGETS)

install:
	$(INSTALLDIR) $(DESTDIR)$(MANDIR)/man8
	$(INSTALLMAN) $(MAN8PAGES) $(DESTDIR)$(MANDIR)/man8

check: all
	@for page in $(MAN8PAGES); do test 0 -eq $$($(MAN_CHECK) $$page \
		$(MAN_REDIRECT)) || { echo "Error in $$page"; exit 1; }; done

.PHONY: install clean distclean check
