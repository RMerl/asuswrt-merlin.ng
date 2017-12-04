include ../common.mak

DIR_NAME=Beceem_BCMS250

CSCM_DIR=CSCM_v1.1.6.0
ifeq ($(RTCONFIG_BCMARM),y)
CROSS_TOOLCHAIN_DIR=$(TOOLCHAIN)
else ifeq ($(RTCONFIG_QCA),y)
CROSS_TOOLCHAIN_DIR=$(TOOLCHAIN)
else
CROSS_TOOLCHAIN_DIR=$(SRCBASE)/../../tools/brcm/hndtools-mipsel-linux
endif
LIBRT=$(shell dirname $(shell which $(CXX)))/../lib/librt.so.0

export CSCM_DIR
export CROSS_TOOLCHAIN_DIR
export LIBRT

ifneq ($(wildcard ./src),)
all:
	@echo " [$(DIR_NAME)] CC"
	make -C src

install:
	@echo " [$(DIR_NAME)] Installing to $(INSTALLDIR)"
	make -C src install

clean:
	make -C src clean

else
all:
	@echo " [$(DIR_NAME)] CC"
	rm -rf $(INSTALLDIR)/$(DIR_NAME)

install:
	@echo " [$(DIR_NAME)] Installing to $(INSTALLDIR)"
	mkdir -p $(INSTALLDIR)
	cp -rf prebuild/lib $(INSTALLDIR)/
	cp -rf prebuild/rom $(INSTALLDIR)/
	cp -rf prebuild/sbin $(INSTALLDIR)/

clean:
	echo "$(INSTALLDIR)"
	rm -rf $(INSTALLDIR)/$(DIR_NAME)
endif

