#
# This is the Makefile for WTFast.
#
# Copyright (C) 2015, AAA Internet Publishing, Inc. (DBA WTFast)
#

include ../common.mak

.PHONY: libgpnutils libgpnclient libev-4.20 gpnrd libwtf wtfd wtfslhd 

#all: libgpnutils libgpnclient libev-4.20 gpnrd libwtf wtfd wtfslhd 
all: libgpnutils libev-4.20 libwtf wtfslhd

libgpnutils:
	$(MAKE) CPPFLAGS="-DDEBUG" -C $@
#	cd $@ && \
#	ln -sf ./$@.so.1.1.0 $@.so.1 && \
#	ln -sf ./$@.so.1.1.0 $@.so

libgpnclient:
	$(MAKE) CPPFLAGS="-DDEBUG -I../libgpnutils -I$(STAGEDIR)/include" \
		LDFLAGS="-L../libgpnutils -L$(STAGEDIR)/usr/lib" \
		-C $@
#	cd $@ && \
#	ln -sf ./$@.so.1.1.0 $@.so.1 && \
#	ln -sf ./$@.so.1.1.0 $@.so

libev-4.20/Makefile:
	cd libev-4.20 && \
	$(CONFIGURE) --host=arm-linux --enable-shared=no

libev-4.20: libev-4.20/Makefile
	$(MAKE) -C $@

libev-4.20-clean:
	[ ! -f libev-4.20/Makefile ] || $(MAKE) -C libev-4.20 distclean
	@rm -f libev-4.20/Makefile

libwtf/Makefile:
	cd libwtf && \
	aclocal; autoreconf -i; $(CONFIGURE) --host=arm-linux --enable-shared=no

libwtf: libwtf/Makefile
	$(MAKE) CPPFLAGS="-I$(STAGEDIR)/include" \
		LDFLAGS="-L$(STAGEDIR)/usr/lib" -C $@

libwtf-clean:
	[ ! -f libwtf/Makefile ] || $(MAKE) -C libwtf distclean
	@rm -f libwtf/Makefile

gpnrd:
	$(MAKE) ROUTER_BUILD=y \
		CPPFLAGS="-DDEBUG -I../libgpnutils -I../libgpnclient \
		    -I$(SRCBASE) -I$(SRCBASE)/include -I$(TOP)/shared -I$(SRCBASE)/shared/bcmwifi/include -I$(SRCBASE)/common/include -I$(STAGEDIR)/include" \
		LDFLAGS="-L../libgpnutils -L../libgpnclient \
		    -L$(TOP)/nvram$(BCMEX)$(EX7) -L$(TOP)/shared -L$(STAGEDIR)/usr/lib" \
		-C $@

wtfd:
	$(MAKE) ROUTER_BUILD=y \
		CPPFLAGS="-I../libgpnutils -I../libev-4.20 -I../libwtf \
		    -I$(SRCBASE) -I$(SRCBASE)/include -I$(TOP)/shared -I$(SRCBASE)/shared/bcmwifi/include -I$(SRCBASE)/common/include -I$(STAGEDIR)/include" \
		LDFLAGS="-L../libgpnutils -L../libev-4.20/.libs/ -L../libwtf/src/.libs \
		    -L$(TOP_PLATFORM)/nvram$(BCMEX)$(EX7) -L$(TOP)/shared -L$(STAGEDIR)/usr/lib" \
		-C $@

wtfslhd:
	$(MAKE) ROUTER_BUILD=y \
		CPPFLAGS="-I../libgpnutils -I../libev-4.20 -I../libwtf \
		    -I$(SRCBASE) -I$(SRCBASE)/include -I$(TOP)/shared -I$(SRCBASE)/shared/bcmwifi/include -I$(SRCBASE)/common/include -I$(STAGEDIR)/include" \
		LDFLAGS="-L../libgpnutils -L../libev-4.20/.libs/ -L../libwtf/src/.libs \
		    -L$(TOP_PLATFORM)/nvram$(BCMEX)$(EX7) -L$(TOP)/shared -L$(STAGEDIR)/usr/lib" \
		-C $@

clean:
	-$(MAKE) -C libgpnutils clean
#	-$(MAKE) -C libgpnclient clean
	-$(MAKE) libev-4.20-clean
#	-$(MAKE) -C gpnrd clean
	-$(MAKE) -C libwtf clean
#	-$(MAKE) -C wtfd clean
	-$(MAKE) -C wtfslhd clean
