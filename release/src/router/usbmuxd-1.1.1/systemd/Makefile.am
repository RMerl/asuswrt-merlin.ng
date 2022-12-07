edit = \
	$(SED) -r \
	-e 's|@sbindir[@]|$(sbindir)|g' \
	-e 's|@localstatedir[@]|$(localstatedir)|g' \
	< $< > $@ || rm $@

if WANT_SYSTEMD

systemdsystemunit_DATA = \
	usbmuxd.service

usbmuxd.service: usbmuxd.service.in
	$(edit)

EXTRA_DIST = \
	usbmuxd.service.in

CLEANFILES = \
	usbmuxd.service

endif
