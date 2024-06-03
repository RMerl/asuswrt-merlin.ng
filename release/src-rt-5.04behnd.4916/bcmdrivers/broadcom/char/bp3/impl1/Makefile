# File: bcmdrivers/broadcom/char/bp3/impl1/Makefile
#
# Makefile for the BP3 Driver
#

obj-$(CONFIG_BCM_LICENSE) += bcm_bp3drv.o 

bcm_bp3drv-objs += bp3drv.o \
	fsaux.o \
	chipid.o \
	utils.o \
	dbg.o

ifeq ("$(CONFIG_BRCM_SMC_BASED)" ,"y")
bcm_bp3drv-objs += bp3smc.o \
	enfsmc.o
else
EXTRA_CFLAGS += -DBP3DRV_LEGACY_MODE
bcm_bp3drv-objs += bp3.o \
	enf.o \
	crypto.o \
	keys.o

MBEDTLS_OBJS = asn1parse.o base64.o bignum.o constant_time.o error.o md.o oid.o pem.o pk.o \
    pkparse.o pk_wrap.o platform_util.o rsa.o rsa_alt_helpers.o sha256.o

bcm_bp3drv-objs += $(addprefix mbedtls/library/, $(MBEDTLS_OBJS))
endif

EXTRA_CFLAGS += $(INC_RDP_FLAGS)
EXTRA_CFLAGS += -I$(INC_BRCMDRIVER_PUB_PATH)/$(BRCM_BOARD) \
		-I$(INC_BRCMSHARED_PUB_PATH)/$(BRCM_BOARD) \
		-I$(INC_BRCMSHARED_PRIV_PATH)/$(BRCM_BOARD) \
		-I$(INC_BRCMDRIVER_PRIV_PATH)/$(BRCM_BOARD) \
		-I$(src)/mbedtls/include

EXTRA_CFLAGS += -Werror -Wfatal-errors

BRCM_RELEASETAG ?= "unknown"
EXTRA_CFLAGS += -DBRCM_RELEASETAG=\"$(BRCM_RELEASETAG)\" \
		-DBRCM_MAINTAINENCETAG=0x$(BRCM_MAINTAINENCETAG)

-include $(TOPDIR)/Rules.make

INSTALL_DONE = $(src)/.done_install

clean-files := $(notdir $(INSTALL_DONE))

$(obj)/bp3drv.o: $(INSTALL_DONE)

$(INSTALL_DONE):
	mkdir -p $(INSTALL_DIR)/etc/licenses
	mkdir -p $(INSTALL_DIR)/etc/init.d
	mkdir -p $(INSTALL_DIR)/etc/rc3.d
	install $(src)/license_init.sh $(INSTALL_DIR)/etc/init.d
	(cd $(INSTALL_DIR)/etc/rc3.d; rm -f S45bcm-c-license; ln -s ../init.d/license_init.sh S45bcm-c-license)
	@touch $@

clean:
	rm -f core *.ko *.o *.a *.s
#ifeq ($(strip $(CONFIG_BCM96888)),)
	rm -f $(MBEDTLS_OBJS)
#endif

# Makefile trick: breaking the following into two rules allows
# the "%.o : %.c" rule (defined earlier by kbuild), to take precidence

%.o : %.o_tmp
	$(Q)mv $< $@

%.o_tmp : %.$(BRCM_CHIP).o_saved
	@echo "Using $<"
	$(Q)cp $< $@
