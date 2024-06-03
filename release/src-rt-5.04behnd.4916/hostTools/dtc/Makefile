
DTC = dtc-2cd89f862cdb04d91c5d59c5b39647f7d5d5b3b8

all:
	tar --skip-old-files -xf $(DTC).tar.gz
	$(MAKE) -C $(DTC) NO_PYTHON=1 STATIC_BUILD=1 EXTRA_CFLAGS=-Wno-error=sign-compare
	cp $(DTC)/{fdtget,fdtput} ../local_install/


clean:
	-[ ! -d $(DTC) ] || $(MAKE) -C $(DTC) clean
	rm -rf $(DTC)
