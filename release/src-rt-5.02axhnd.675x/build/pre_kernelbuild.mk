
default: $(KERNEL_DIR)/.pre_kernelbuild

include $(BUILD_DIR)/make.common

define kernel_cfg_rm_bcm_kf
	sed -i.bak -e "/^CONFIG_BCM_.*=[my]/d" $(KERNEL_DIR)/.config && \
	sed -i.bak -e "/default [my]/d" $(BCM_KF_KCONFIG_FILE) && \
	CURRENT_ARCH=$(KERNEL_ARCH) TOOLCHAIN_TOP= $(MAKE) -C $(BUILD_DIR)/build -f Bcmkernel.mk olddefconfig
endef

define android_kernel_merge_cfg
cd $(KERNEL_DIR); \
ARCH=${ARCH} scripts/kconfig/merge_config.sh -m arch/$(ARCH)/defconfig android/configs/android-base.cfg android/configs/android-recommended.cfg android/configs/android-bcm-recommended.cfg ;
endef

$(KERNEL_DIR)/.pre_kernelbuild: $(BCM_KF_KCONFIG_FILE)
	@echo
	@echo -------------------------------------------
	@echo ... starting kernel build at $(KERNEL_DIR)
	@echo PROFILE_KERNEL_VER is $(PROFILE_KERNEL_VER)
	@echo BCM_KF is $(if $(BCM_KF),,un)defined
	@cd $(INC_KERNEL_BASE); \
	if [ ! -e $(KERNEL_DIR)/.untar_complete ]; then \
		echo "Untarring original Linux kernel source: $(LINUX_ZIP_FILE)"; \
		(tar xkfpj $(LINUX_ZIP_FILE) 2> /dev/null || true); \
		touch $(KERNEL_DIR)/.untar_complete; \
	fi && \
	#$(GENDEFCONFIG_CMD) $(PROFILE_PATH) ${MAKEFLAGS} && \
	#cp -f $(KERNEL_DIR)/arch/$(ARCH)/defconfig $(KERNEL_DIR)/.config && \
	#$(if $(strip $(BRCM_ANDROID)), $(call android_kernel_merge_cfg), true) && \
	$(MAKE) -C $(BUILD_DIR)/build -f Bcmkernel.mk olddefconfig && \
	$(if $(BCM_KF), true, $(call kernel_cfg_rm_bcm_kf)) && \
	touch $(KERNEL_DIR)/.pre_kernelbuild; 
