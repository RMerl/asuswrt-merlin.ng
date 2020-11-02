# File: bcmdrivers/broadcom/char/license/impl1/Makefile
#
# Makefile for the License driver.
#

BCMARCH=$(subst ",,$(CONFIG_BCM_CPU_ARCH_NAME))

obj-$(CONFIG_BCM_LICENSE) += bcm_license.o
bcm_license-objs += license.o parser.o decrypt.o util.o
ifneq ($(findstring _$(strip $(BRCM_CHIP))_,_63138_63148_),)
bcm_license-objs += lutil.o
endif

EXTRA_CFLAGS += $(INC_RDP_FLAGS)
EXTRA_CFLAGS += -I$(INC_BRCMDRIVER_PUB_PATH)/$(BRCM_BOARD) -I$(INC_BRCMSHARED_PUB_PATH)/$(BRCM_BOARD)
EXTRA_CFLAGS += -Werror -Wfatal-errors -DBRCM_RELEASETAG=\"$(BRCM_RELEASETAG)\"
EXTRA_CFLAGS += -I$(INC_BRCMDRIVER_PRIV_PATH)/$(BRCM_BOARD)

-include $(TOPDIR)/Rules.make

$(obj)/license.o: install

# Changed loading rc script to S45bcm-c-license
# so that license module receives license file early enough to serve other kernel modules.
install:
	mkdir -p $(INSTALL_DIR)/etc/init.d
	mkdir -p $(INSTALL_DIR)/etc/rc3.d
	mkdir -p $(INSTALL_DIR)/etc/licenses
	mkdir -p $(INSTALL_DIR)/data/licenses
	if [ -a $(src)/scripts/license.txt ]; then install $(src)/scripts/license.txt $(INSTALL_DIR)/etc/licenses; fi;
	install $(src)/scripts/license_init.sh $(INSTALL_DIR)/etc/init.d
	(cd $(INSTALL_DIR)/etc/rc3.d; rm -f S45bcm-c-license; ln -s ../init.d/license_init.sh S45bcm-c-license)


clean:
	rm -f core *.ko *.o *.a *.s


# Makefile trick: breaking the following into two rules allows
# the "%.o : %.c" rule (defined earlier by kbuild), to take precidence

%.o : %.o_tmp
	$(Q)mv $< $@

%.o_tmp : %.$(BCMARCH).o_saved
	@echo "Using $<"
	$(Q)cp $< $@
