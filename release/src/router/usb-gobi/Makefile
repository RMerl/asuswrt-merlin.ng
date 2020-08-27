include ../common.mak

GOBI_LIB_VER=00.00.05

NET_USB_GOBI = $(INSTALLDIR)/lib/modules/$(LINUX_KERNEL)/kernel/drivers/net/usb/
ifeq ($(RTCONFIG_BCMARM),y)
GOBI_LIB_DIR=lib/bcm.arm
LIBSTDC=$(shell dirname $(shell which $(CXX)))/../arm-brcm-linux-uclibcgnueabi/lib/libstdc++.so.6
NET_USB_GOBI = $(INSTALLDIR)/lib/modules/2.6.36.4brcmarm/kernel/drivers/net/usb/
else # QCA
ifeq ($(RTCONFIG_SOC_IPQ40XX),y)
GOBI_LIB_DIR=lib/ipq40xx
else
GOBI_LIB_DIR=lib/qca95xx
endif
LIBSTDC=$(shell dirname $(shell which $(CXX)))/../lib/libstdc++.so.6
endif
ifeq ($(UCLIBC_NG), y)
LIBRT=$(shell dirname $(shell which $(CXX)))/../lib/librt.so.1
else
LIBRT=$(shell dirname $(shell which $(CXX)))/../lib/librt.so.0
endif

export GOBI_LIB_DIR

ifeq ($(wildcard src),)
GOBI_KO_DIR = prebuild
GOBI_UTIL_DIR = prebuild
GOBI_LIB_DIR = .
else
PWD = $(shell pwd)
MODULE_DIR = $(PWD)/kernel_module/gobi
GOBI_KO_DIR = $(MODULE_DIR)
GOBI_UTIL_DIR = src
endif


GOBI_KO = $(GOBI_KO_DIR)/gobi.ko
GOBI_UTIL = $(GOBI_UTIL_DIR)/gobi_api
LIB_GOBI_CONN_MGMT = libGobiConnectionMgmt.$(GOBI_LIB_VER).so


ifeq ($(wildcard src),)
all:
clean:
	@echo "do nothing"
else
all: $(GOBI_KO) $(GOBI_UTIL)

$(GOBI_KO):
	$(MAKE) -C $(LINUXDIR) SUBDIRS=$(MODULE_DIR) modules CONFIG_USB_NET_GOBI=m

$(GOBI_UTIL):
	$(MAKE) -C src

clean:
	$(MAKE) -C $(LINUXDIR) SUBDIRS=$(MODULE_DIR) clean
	$(MAKE) -C src clean
endif

install: $(GOBI_KO) $(GOBI_UTIL)
	@rm -rf $(INSTALLDIR)
	@install -D $(GOBI_KO) $(NET_USB_GOBI)/gobi.ko
ifeq ($(UCLIBC_NG), y)
	@install -D $(LIBRT) $(INSTALLDIR)/lib/librt.so.1
else
	@install -D $(LIBRT) $(INSTALLDIR)/lib/librt.so.0
endif
	@install -D $(LIBSTDC) $(INSTALLDIR)/usr/lib/libstdc++.so.6
	@install -D $(GOBI_UTIL_DIR)/$(GOBI_LIB_DIR)/$(LIB_GOBI_CONN_MGMT) $(INSTALLDIR)/lib/$(LIB_GOBI_CONN_MGMT)
	@cd $(INSTALLDIR)/lib/ && ln -s $(LIB_GOBI_CONN_MGMT) libGobiConnectionMgmt.so
	@install -D $(GOBI_UTIL_DIR)/sample $(INSTALLDIR)/usr/sbin/sample
	@$(STRIP) $(INSTALLDIR)/usr/sbin/sample
	install -D $(GOBI_UTIL) $(INSTALLDIR)/usr/sbin/gobi_api
	$(STRIP) $(INSTALLDIR)/usr/sbin/gobi_api
	#@install $(GOBI_UTIL_DIR)/open_tty $(INSTALLDIR)/usr/sbin/open_tty
	#@$(STRIP) $(INSTALLDIR)/usr/sbin/open_tty


.PHONY: all clean install
.PHONY: $(GOBI_KO) $(GOBI_UTIL)

