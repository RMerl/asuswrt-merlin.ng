default_bld:all

all: libwlcsm.so

libwlcsm.so:
	cp -f $(TOP_PLATFORM)/$(shell pwd | sed 's/\(.*router-sysdep\/\)//')/$(PRBM_DIR)/$@ ./
	install -m 755 libwlcsm.so $(INSTALL_DIR)/lib

install: all
	install -m 755 libwlcsm.so $(INSTALL_DIR)/lib

distclean clean:
	-rm -f libwlcsm.so
