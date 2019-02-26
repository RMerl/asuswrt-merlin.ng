# make this relative to the location of strongswan.conf
strongswanconfdir = `dirname $(strongswan_conf)`
strongswanddir = $(strongswanconfdir)/strongswan.d
charonconfdir = $(strongswanddir)/charon
# copy these files also to /usr/share
templatesdir = $(pkgdatadir)/templates/config
optionstemplatedir = $(templatesdir)/strongswan.d
pluginstemplatedir = $(templatesdir)/plugins

options = \
	options/aikgen.opt \
	options/attest.opt \
	options/charon.opt \
	options/charon-logging.opt \
	options/charon-nm.opt \
	options/charon-systemd.opt \
	options/imcv.opt \
	options/imv_policy_manager.opt \
	options/manager.opt \
	options/medsrv.opt \
	options/pki.opt \
	options/pool.opt \
	options/scepclient.opt \
	options/starter.opt \
	options/swanctl.opt \
	options/tnc.opt \
	options/sec-updater.opt \
	options/sw-collector.opt

plugins = \
	plugins/addrblock.opt \
	plugins/android_log.opt \
	plugins/attr.opt \
	plugins/attr-sql.opt \
	plugins/bliss.opt \
	plugins/bypass-lan.opt \
	plugins/certexpire.opt \
	plugins/coupling.opt \
	plugins/curl.opt \
	plugins/dhcp.opt \
	plugins/dnscert.opt \
	plugins/duplicheck.opt \
	plugins/eap-aka.opt \
	plugins/eap-aka-3gpp.opt \
	plugins/eap-aka-3gpp2.opt \
	plugins/eap-dynamic.opt \
	plugins/eap-gtc.opt \
	plugins/eap-peap.opt \
	plugins/eap-radius.opt \
	plugins/eap-sim.opt \
	plugins/eap-simaka-sql.opt \
	plugins/eap-tls.opt \
	plugins/eap-tnc.opt \
	plugins/eap-ttls.opt \
	plugins/error-notify.opt \
	plugins/ext-auth.opt \
	plugins/forecast.opt \
	plugins/gcrypt.opt \
	plugins/ha.opt \
	plugins/imc-attestation.opt \
	plugins/imc-hcd.opt \
	plugins/imc-os.opt \
	plugins/imc-scanner.opt \
	plugins/imc-swima.opt \
	plugins/imc-test.opt \
	plugins/imv-attestation.opt \
	plugins/imv-os.opt \
	plugins/imv-scanner.opt \
	plugins/imv-swima.opt \
	plugins/imv-test.opt \
	plugins/ipseckey.opt \
	plugins/led.opt \
	plugins/kernel-libipsec.opt \
	plugins/kernel-netlink.opt \
	plugins/kernel-pfkey.opt \
	plugins/kernel-pfroute.opt \
	plugins/load-tester.opt \
	plugins/lookip.opt \
	plugins/ntru.opt \
	plugins/openssl.opt \
	plugins/osx-attr.opt \
	plugins/p-cscf.opt \
	plugins/pkcs11.opt \
	plugins/radattr.opt \
	plugins/random.opt \
	plugins/resolve.opt \
	plugins/revocation.opt \
	plugins/save-keys.opt \
	plugins/socket-default.opt \
	plugins/sql.opt \
	plugins/stroke.opt \
	plugins/systime-fix.opt \
	plugins/tnc-ifmap.opt \
	plugins/tnc-imc.opt \
	plugins/tnc-imv.opt \
	plugins/tnc-pdp.opt \
	plugins/tnccs-11.opt \
	plugins/tnccs-20.opt \
	plugins/tpm.opt \
	plugins/unbound.opt \
	plugins/updown.opt \
	plugins/vici.opt \
	plugins/whitelist.opt \
	plugins/xauth-eap.opt \
	plugins/xauth-pam.opt

alloptions = $(options) $(plugins)

confsnippets = $(alloptions:opt=conf)

# we only install snippets for enabled plugins
plugins_install_tmp = $(charon_plugins:%=plugins/%.tmp)
plugins_install_src = $(charon_plugins:%=plugins/%.conf)
# only install snippets for enabled components
# has to be defined via autoconf as we can't do it with automake conditionals
options_install_src = $(strongswan_options:%=options/%.conf)

templates_DATA = strongswan.conf
optionstemplate_DATA = $(options_install_src)
pluginstemplate_DATA = $(plugins_install_src)
man_MANS = \
	strongswan.conf.5

BUILT_SOURCES = default.conf strongswan.conf.5.main $(confsnippets)
EXTRA_DIST = format-options.py strongswan.conf default.opt \
	default.conf strongswan.conf.5.main $(alloptions) $(confsnippets)

CLEANFILES=$(man_MANS)

.opt.conf:
	$(AM_V_GEN) \
	case "$<" in \
	*plugins/*) \
		sed \
		-e "s:\@PLUGIN_NAME\@:`basename $< .opt`:" \
		$(srcdir)/default.opt | cat - $< | \
		$(PYTHON) $(srcdir)/format-options.py -f conf -r charon.plugins > $(srcdir)/$@ \
		;; \
	*) \
		$(PYTHON) $(srcdir)/format-options.py -f conf -r charon.plugins $< > $(srcdir)/$@ \
		;; \
	esac

# we need another implicit rule to generate files from the generic template only
# if the rules above did not catch it.  this requires an intermediate step that
# generates a copy of the generic config template.
$(plugins_install_tmp):
	@mkdir -p $(builddir)/plugins
	@cp $(srcdir)/default.conf $(builddir)/$@

.tmp.conf:
	$(AM_V_GEN) \
	sed \
	-e "s:\@PLUGIN_NAME\@:`basename $< .tmp`:" \
	$(builddir)/$< > $(builddir)/$@

strongswan.conf.5.main: $(alloptions)
	$(AM_V_GEN) \
	cd $(srcdir) && $(PYTHON) format-options.py -f man $(alloptions) > $@

strongswan.conf.5: strongswan.conf.5.head strongswan.conf.5.main strongswan.conf.5.tail
	$(AM_V_GEN) \
	cat strongswan.conf.5.head $(srcdir)/strongswan.conf.5.main strongswan.conf.5.tail > $@

clean-local:
	rm -f plugins/*.conf plugins/*.tmp

maintainer-clean-local:
	cd $(srcdir) && \
		rm -f $(confsnippets) default.conf plugins/*.conf plugins/*.tmp

install-data-local: $(plugins_install_src)
	test -e "$(DESTDIR)${strongswanconfdir}" || $(INSTALL) -d "$(DESTDIR)$(strongswanconfdir)" || true
	test -e "$(DESTDIR)${strongswanddir}" || $(INSTALL) -d "$(DESTDIR)$(strongswanddir)" || true
	test -e "$(DESTDIR)${charonconfdir}" || $(INSTALL) -d "$(DESTDIR)$(charonconfdir)" || true
	test -e "$(DESTDIR)$(strongswanconfdir)/strongswan.conf" || $(INSTALL) -m 644 $(srcdir)/strongswan.conf $(DESTDIR)$(strongswanconfdir)/strongswan.conf || true
	for f in $(options_install_src); do \
		name=`basename $$f`; \
		test -f "$(DESTDIR)$(strongswanddir)/$$name" || $(INSTALL) -m 644 "$(srcdir)/$$f" "$(DESTDIR)$(strongswanddir)/$$name" || true; \
	done
	for f in $(plugins_install_src); do \
		name=`basename $$f`; \
		if test -f "$$f"; then dir=; else dir="$(srcdir)/"; fi; \
		test -f "$(DESTDIR)$(charonconfdir)/$$name" || $(INSTALL) -m 644 "$$dir$$f" "$(DESTDIR)$(charonconfdir)/$$name" || true; \
	done
