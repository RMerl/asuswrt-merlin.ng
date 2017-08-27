EXTRA_DIST = vici.gemspec.in lib/vici.rb

vici.gemspec: $(srcdir)/vici.gemspec.in
	$(AM_V_GEN) sed \
	-e "s:@GEM_VERSION@:$(PACKAGE_VERSION):" \
	$(srcdir)/vici.gemspec.in > $@

vici-$(PACKAGE_VERSION).gem: vici.gemspec
	$(GEM) build vici.gemspec

all-local: vici-$(PACKAGE_VERSION).gem

clean-local:
	rm -f vici.gemspec vici-$(PACKAGE_VERSION).gem

install-data-local: vici-$(PACKAGE_VERSION).gem
	$(GEM) install --install-dir $(DESTDIR)$(RUBYGEMDIR) \
		vici-$(PACKAGE_VERSION).gem

uninstall-local:
	$(GEM) uninstall --install-dir $(DESTDIR)$(RUBYGEMDIR) \
		--version $(PACKAGE_VERSION) vici
