
EXTRA_DIST = strongswan-starter.service.in
CLEANFILES = strongswan-starter.service

systemdsystemunit_DATA = strongswan-starter.service

strongswan-starter.service : strongswan-starter.service.in
	$(AM_V_GEN) \
	sed \
	-e "s:@SBINDIR@:$(sbindir):" \
	-e "s:@IPSEC_SCRIPT@:$(ipsec_script):" \
	$(srcdir)/$@.in > $@
