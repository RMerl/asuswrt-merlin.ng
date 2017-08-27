
EXTRA_DIST = strongswan-swanctl.service.in
CLEANFILES = strongswan-swanctl.service

systemdsystemunit_DATA = strongswan-swanctl.service

strongswan-swanctl.service : strongswan-swanctl.service.in
	$(AM_V_GEN) \
	sed \
	-e "s:@SBINDIR@:$(sbindir):" \
	$(srcdir)/$@.in > $@
