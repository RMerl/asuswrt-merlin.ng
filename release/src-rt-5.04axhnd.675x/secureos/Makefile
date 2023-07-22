# Makefile for secure OS build
ATF_DIR := $(BUILD_DIR)/secureos
SECURE_OS_DIR := $(BUILD_DIR)/secureos

ifneq ($(strip $(BUILD_SECURE_OS)),y)
optee optee_clean : ;
endif


############################################################################
# OP-TEE targets
############################################################################
ifeq ($(strip $(BCM_OPTEE)),y)

optee_os:
	$(MAKE) -C $(SECURE_OS_DIR)/optee
	echo "Compress OPTEE using lzma"
	$(BUILD_DIR)/hostTools/cmplzma -k -2 -lzma $(BUILD_DIR)/secureos/optee/tee.elf $(BUILD_DIR)/secureos/optee/tee.bin $(BUILD_DIR)/secureos/optee/optee.lz

clean:
	$(MAKE) -C $(SECURE_OS_DIR)/optee distclean
endif # BCM_OPTEE


