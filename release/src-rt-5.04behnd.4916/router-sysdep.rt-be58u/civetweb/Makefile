SRC := civetweb

LIB_DIR := civetweb
PREFIX := $(BCM_FSBUILD_DIR)/public
HEADER_INSTALL_DIR := $(BCM_FSBUILD_DIR)/public/include

install:
	cd $(LIB_DIR) ; $(MAKE) lib && $(MAKE) install-lib PREFIX=$(PREFIX)
	cp -d $(LIB_DIR)/include/civetweb.h $(HEADER_INSTALL_DIR)

clean:
	cd $(SRC)
	rm -f *so*
	rm -f *.a
