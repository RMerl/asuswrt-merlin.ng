export MSIPK
export SRCDIR
export CURRENT
all:
	$(MAKE) -C src

install:
ifneq ($(MSIPK),y)
	$(MAKE) -C src romfs
	install -d $(INSTALLDIR)/rom/etc
	cp -rf admin-root $(INSTALLDIR)/rom/etc/web 
else
	install -D $(SRCDIR)/mt-daapd/src/mt-daapd $(CURRENT)/opt/sbin/mt-daapd 
	cp -f $(CURRENT)/opt/sbin/mt-daapd $(CURRENT)/mediaserver/opt/tmp/bin/
endif

clean:
	$(MAKE) -C src clean
