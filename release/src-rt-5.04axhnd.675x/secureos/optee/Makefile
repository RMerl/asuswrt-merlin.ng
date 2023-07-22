# OPTEE Makefile
OPTEE_VER := 3.7.0
OPTEE_TAR := $(OPTEE_VER).tar.gz
OPTEE_DIR := optee_os-$(OPTEE_VER)
OPTEE_OUT := $(OPTEE_DIR)/out/arm-plat-bca
OPTEE_APPS := $(patsubst apps/%/,%,$(shell ls -d apps/*/))
OPTEE_PTA := $(patsubst pta/%/,%,$(shell ls -d pta/*/))
OPTEE_PLATS := $(patsubst plat/%/,%,$(shell ls -d plat/*/))
OPTEE_DRV := $(patsubst drivers/%/,%,$(shell ls -d drivers/*/))
OPTEE_SCRIPT := $(shell find ./optee_os-$(OPTEE_VER)/[sc]* -name *.py)
PATH_TO_ENV := $(subst /,\/,$(shell which env))

ifneq ($(findstring _$(strip $(BRCM_CHIP))_,_63138_63178_6846_47622_6756_),)
OPTEE_ARCH := arm32
else
OPTEE_ARCH := arm64
endif

OPTEE_BUILD_PARMS = PLATFORM=bca \
                    PLATFORM_DIR=$(BRCM_CHIP) \
                    PLATFORM_FLAVOR=$(BRCM_CHIP) \
                    CROSS_COMPILE=$(KCROSS_COMPILE) \
                    TA_DEV_KIT_DIR=$(BUILD_DIR)/secureos/optee/$(OPTEE_OUT)/export-ta_$(OPTEE_ARCH)

OPTEE_APP_DIR  := $(OPTEE_DIR)/../apps/early_ta
OPTEE_APP_PATH := $(BUILD_DIR)/secureos/optee/$(OPTEE_APP_DIR)
# List TA's UUIDs to be part of the final image
OPTEE_APP_UUID := "$(OPTEE_APP_PATH)/8aaaf200-2450-11e4-abe2-0002a5d5c51b.stripped.elf \
                   $(OPTEE_APP_PATH)/8aaaf201-2450-11e4-abe2-0002a5d5c51b.stripped.elf "

Q := @
ifneq ($(strip $(VERBOSE)),)
Q :=
endif

# ASUSWRT
unexport CFLAGS

.PHONY: all optee clean

all: tee.bin

include $(BUILD_DIR)/make.common


$(OPTEE_DIR)/Makefile: $(OPTEE_TAR) $(OPTEE_VER).patch Makefile
	$(Q)rm -rf $(OPTEE_DIR)
	@echo "Untarring source and overrides..."
	$(Q)tar xf $(OPTEE_TAR) 2> /dev/null
	@echo "Applying patches to OPTEE..."
	$(Q)patch -p1 -b -N -s -d$(OPTEE_DIR) < $(OPTEE_VER).patch
	$(Q)for plat in $(OPTEE_PLATS); do \
		echo "Link platform $${plat}" ;\
		ln -s ../../../../plat/$${plat} $(OPTEE_DIR)/core/arch/arm/$${plat} ; \
	done
	@echo "Replacing to env path inside python script..."
	$(Q)for script in $(OPTEE_SCRIPT); do \
		sed -i 's/\/usr\/bin\/env/$(PATH_TO_ENV)/g' $${script} ; \
	done
	@echo "Adding drivers: $(OPTEE_DRV)"
	$(Q)echo "subdirs-y += $(addprefix ../../../drivers/,$(OPTEE_DRV))" >> $(OPTEE_DIR)/core/drivers/sub.mk
	@echo "Adding pta: $(OPTEE_PTA)"
	$(Q)echo "subdirs-y += $(addprefix ../../../pta/,$(OPTEE_PTA))" >> $(OPTEE_DIR)/core/pta/sub.mk
ifneq ($(findstring _$(strip $(BRCM_CHIP))_,_63138_),)
	cp $(OPTEE_DIR)/../plat/plat-bca/bcm63138/arm32_sysreg.txt $(OPTEE_DIR)/core/arch/arm/kernel/arm32_sysreg.txt
endif
	$(Q)touch $@


optee: $(OPTEE_DIR)/Makefile
	$(Q)$(MAKE) -C $(OPTEE_DIR) $(OPTEE_BUILD_PARMS) CFG_EARLY_TA=y ta_dev_kit
# Build all the TAs
	$(Q)$(MAKE) -C $(OPTEE_DIR)/../apps/teec_hello $(OPTEE_BUILD_PARMS) CFG_EARLY_TA=y
	$(Q)$(MAKE) -C $(OPTEE_DIR)/../apps/ta_sec_key $(OPTEE_BUILD_PARMS) CFG_EARLY_TA=y

# Build a directory to keep all the early TAs
	$(Q)rm -fr $(OPTEE_APP_DIR)
	$(Q)mkdir $(OPTEE_APP_DIR)
# Dump all the early TAs into that directory
	$(Q)cp -f $(OPTEE_DIR)/../apps/teec_hello/*.stripped.elf                      $(OPTEE_APP_DIR)
	$(Q)cp -f $(OPTEE_DIR)/../apps/ta_sec_key/*.stripped.elf                      $(OPTEE_APP_DIR)

# Pack all the early TAs at the end of the final image
	$(Q)$(MAKE) -C $(OPTEE_DIR) $(OPTEE_BUILD_PARMS) EARLY_TA_PATHS=$(OPTEE_APP_UUID)



tee.bin: optee
	$(Q)install -m 0644 $(OPTEE_OUT)/core/tee.bin tee.bin
	$(Q)install -m 0644 $(OPTEE_OUT)/core/tee.elf tee.elf

clean:
	if [ -d $(OPTEE_DIR) ]; then \
		$(MAKE) -C $(OPTEE_DIR)/../apps/teec_hello $(OPTEE_BUILD_PARMS) clean; \
		$(MAKE) -C $(OPTEE_DIR)/../apps/ta_sec_key $(OPTEE_BUILD_PARMS) clean; \
		$(MAKE) -C $(OPTEE_DIR) $(OPTEE_BUILD_PARMS) clean; \
		find . -type f -name "*.[ado]" \
			-o -name "*.bin" \
			-o -name "*.elf" \
			-o -name "*.cmd" \
			-o -name "*.map" \
			-o -name "*.dmp" \
			-o -name "*.ta" \
			-o -name "*.lds" \
			| xargs rm -f; \
		rm -f $(BUILD_DIR)/secureos/optee/*.lz; \
		rm -f $(TARGET_BOOTFS)/optee.dtb; \
	fi

distclean: clean
	@rm -rf $(OPTEE_DIR)
