include ../common.mak

all:

install:
	install -d $(INSTALLDIR)/sbin
	install -d $(INSTALLDIR)/usr/sbin
	install -d $(INSTALLDIR)/lib
	install -d $(INSTALLDIR)/usr/bin
	install -d $(INSTALLDIR)/usr/share
	install -d $(INSTALLDIR)/usr/lib
	install -d $(INSTALLDIR)/rom/etc
ifeq ($(HND_ROUTER), y)
	install -d $(INSTALLDIR)/www/www
	install -d $(INSTALLDIR)/www/www/js
	ln -snrf $(INSTALLDIR)/jffs/.koolshare/res $(INSTALLDIR)/www/www/res
else
	install -d $(INSTALLDIR)/www
	install -d $(INSTALLDIR)/www/js
	ln -snrf $(INSTALLDIR)/jffs/.koolshare/res $(INSTALLDIR)/www/res
endif
ifeq ($(BUILD_NAME), $(filter $(BUILD_NAME), GT-AC2900 GT-AC5300))
	install -d $(INSTALLDIR)/rom/etc/koolshare/ROG/res
	install -d $(INSTALLDIR)/rom/etc/koolshare/ROG/webs
else ifeq  ($(BUILD_NAME), $(filter $(BUILD_NAME), TUF-AX3000))
	install -d $(INSTALLDIR)/rom/etc/koolshare/TUF/res
	install -d $(INSTALLDIR)/rom/etc/koolshare/TUF/webs
endif
# softcenter
	mkdir -p $(INSTALLDIR)/rom/etc/koolshare
	cp ./softcenter/. $(INSTALLDIR)/rom/etc/koolshare/ -rf
ifeq ($(BUILD_NAME), $(filter $(BUILD_NAME), GT-AC2900 GT-AC5300))
	install -m 0755 softcenter/res/softcenter.css $(INSTALLDIR)/rom/etc/koolshare/ROG/res
	install -m 0755 softcenter/webs/Module_Softcenter.asp $(INSTALLDIR)/rom/etc/koolshare/ROG/webs
	install -m 0755 softcenter/webs/Module_Softsetting.asp $(INSTALLDIR)/rom/etc/koolshare/ROG/webs
else ifeq  ($(BUILD_NAME), $(filter $(BUILD_NAME), TUF-AX3000))
	install -m 0755 softcenter/res/softcenter.css $(INSTALLDIR)/rom/etc/koolshare/TUF/res
	install -m 0755 softcenter/webs/Module_Softcenter.asp $(INSTALLDIR)/rom/etc/koolshare/TUF/webs
	install -m 0755 softcenter/webs/Module_Softsetting.asp $(INSTALLDIR)/rom/etc/koolshare/TUF/webs
endif
	install -m 0755 jffsinit.sh $(INSTALLDIR)/usr/bin
	install -m 0755 koolshare-reset $(INSTALLDIR)/usr/bin
	ln -snrf $(INSTALLDIR)/jffs/.koolshare $(INSTALLDIR)/koolshare

clean:

