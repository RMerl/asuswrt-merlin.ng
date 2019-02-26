sbin_SCRIPTS = _ipsec
CLEANFILES = _ipsec _ipsec.8
dist_man8_MANS = _ipsec.8
EXTRA_DIST = _ipsec.in _ipsec.8.in Android.mk

_ipsec.8 : _ipsec.8.in
	$(AM_V_GEN) \
	sed \
	-e "s:@IPSEC_VERSION@:$(PACKAGE_VERSION):" \
	-e "s:@IPSEC_SCRIPT@:$(ipsec_script):g" \
	-e "s:@IPSEC_SCRIPT_UPPER@:$(ipsec_script_upper):g" \
	-e "s:@IPSEC_DIR@:$(ipsecdir):" \
	$(srcdir)/$@.in > $@

_ipsec : _ipsec.in
	$(AM_V_GEN) \
	sed \
	-e "s:@IPSEC_SHELL@:/bin/sh:" \
	-e "s:@IPSEC_VERSION@:$(PACKAGE_VERSION):" \
	-e "s:@IPSEC_NAME@:$(PACKAGE_NAME):" \
	-e "s:@IPSEC_DISTRO@::" \
	-e "s:@IPSEC_DIR@:$(ipsecdir):" \
	-e "s:@IPSEC_SCRIPT@:$(ipsec_script):" \
	-e "s:@IPSEC_BINDIR@:$(bindir):" \
	-e "s:@IPSEC_SBINDIR@:$(sbindir):" \
	-e "s:@IPSEC_CONFDIR@:$(sysconfdir):" \
	-e "s:@IPSEC_PIDDIR@:$(piddir):" \
	$(srcdir)/$@.in > $@
	chmod +x $@

install-exec-hook:
	mv $(DESTDIR)$(sbindir)/_ipsec $(DESTDIR)$(sbindir)/$(ipsec_script)

install-data-hook:
	mv $(DESTDIR)$(man8dir)/_ipsec.8 $(DESTDIR)$(man8dir)/$(ipsec_script).8

uninstall-hook:
	rm -f $(DESTDIR)$(sbindir)/$(ipsec_script)
	rm -f $(DESTDIR)$(man8dir)/$(ipsec_script).8
