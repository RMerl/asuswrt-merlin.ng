include_HEADERS = usb.h
lib_LTLIBRARIES = libusb.la

libusb_la_SOURCES = core.c usbi.h libusb-dload.h
libusb_la_CFLAGS = -fvisibility=hidden $(AM_CFLAGS) $(LIBUSB_1_0_CFLAGS)
if USE_LIBUSB_1_0_DLOAD
libusb_la_LIBADD = -ldl
else
libusb_la_LIBADD = $(LIBUSB_1_0_LIBS)
endif
libusb_la_LDFLAGS = -no-undefined -version-info $(LT_MAJOR):$(LT_REVISION):$(LT_AGE) \
	-release 0.1

