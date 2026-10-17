
PWD := $(dir $(lastword $(MAKEFILE_LIST)))
REVISION ?= $(patsubst v%.tar.gz,%,$(notdir $(wildcard $(PWD)v*.tar.gz)))
OVPN_DCO_TGZ:=v$(REVISION).tar.gz
OVPN_DCO_SRC := ovpn-dco-$(REVISION)

RM := rm -rf

CFLAGS = $(EXTRA_CFLAGS)

NOSTDINC_FLAGS += \
	-I$(PWD)$(OVPN_DCO_SRC)/include/ \
	$(CFLAGS) \
	-include $(PWD)$(OVPN_DCO_SRC)/linux-compat.h \
	-I$(PWD)$(OVPN_DCO_SRC)/compat-include/

NOSTDINC_FLAGS += -DOVPN_DCO_VERSION=\"$(REVISION)\"

obj-$(CONFIG_BCM_OPENVPN_DCO) += $(OVPN_DCO_SRC)/drivers/net/ovpn-dco/

BUILD_FLAGS := M=$(PWD) PWD=$(PWD) REVISION=$(REVISION)

default: config
	$(MAKE) -C $(KERNEL_DIR) $(BUILD_FLAGS)	modules

clean:
	$(RM) $(PWD)$(OVPN_DCO_SRC)/compat-autoconf.h*
	$(MAKE) -C $(KERNEL_DIR) $(BUILD_FLAGS) clean

install: config
	$(MAKE) -C $(KERNEL_DIR) $(BUILD_FLAGS) modules_install

config:
	$(PWD)$(OVPN_DCO_SRC)/gen-compat-autoconf.sh $(PWD)$(OVPN_DCO_SRC)/compat-autoconf.h

prepare_kernel_pkgs:
	@if [ ! -e $(PWD).ovpn_dco_untar_complete ]; then \
		echo "Untarring original openvpn dco $(OVPN_DCO_TGZ)"; \
		(tar -xkpzf $(OVPN_DCO_TGZ) 2> /dev/null || true); \
		patch -d $(PWD)$(OVPN_DCO_SRC)/drivers/net/ovpn-dco \
			-p1 < $(PWD)bcm_ovpn_dco.patch; \
		touch $(PWD).ovpn_dco_untar_complete; \
	fi

clean_kernel_pkgs:
	$(RM) $(PWD)$(OVPN_DCO_SRC) $(PWD).ovpn_dco_untar_complete

.PHONY: default clean install config prepare_kernel_pkgs clean_kernel_pkgs
