AM_CPPFLAGS = \
	-I$(top_srcdir)/include \
	-I$(top_srcdir)

AM_CFLAGS = \
	$(GLOBAL_CFLAGS) \
	$(libplist_CFLAGS) \
	$(libusb_CFLAGS) \
	$(libimobiledevice_CFLAGS)

AM_LDFLAGS = \
	$(libplist_LIBS) \
	$(libusb_LIBS) \
	$(libimobiledevice_LIBS) \
	$(libpthread_LIBS)

sbin_PROGRAMS = usbmuxd

usbmuxd_CFLAGS = $(AM_CFLAGS)
usbmuxd_LDFLAGS = $(AM_LDFLAGS) -no-undefined
usbmuxd_SOURCES = \
	client.c client.h \
	device.c device.h \
	preflight.c preflight.h \
	log.c log.h \
	usbmuxd-proto.h \
	usb.c usb.h \
	utils.c utils.h \
	conf.c conf.h \
	main.c
