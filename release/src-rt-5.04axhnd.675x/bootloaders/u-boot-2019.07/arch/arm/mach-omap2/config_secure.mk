# SPDX-License-Identifier: GPL-2.0+
#
# Copyright (C) 2016, Texas Instruments, Incorporated - http://www.ti.com/
quiet_cmd_mkomapsecimg = SECURE  $@
ifneq ($(TI_SECURE_DEV_PKG),)
ifneq ($(wildcard $(TI_SECURE_DEV_PKG)/scripts/create-boot-image.sh),)
ifneq ($(CONFIG_SPL_BUILD),)
cmd_mkomapsecimg = $(TI_SECURE_DEV_PKG)/scripts/create-boot-image.sh \
	$(patsubst u-boot-spl_HS_%,%,$(@F)) $< $@ $(CONFIG_ISW_ENTRY_ADDR) \
	$(if $(KBUILD_VERBOSE:1=), >/dev/null)
else
cmd_mkomapsecimg = $(TI_SECURE_DEV_PKG)/scripts/create-boot-image.sh \
	$(patsubst u-boot_HS_%,%,$(@F)) $< $@ $(CONFIG_ISW_ENTRY_ADDR) \
	$(if $(KBUILD_VERBOSE:1=), >/dev/null)
endif
else
cmd_mkomapsecimg = echo "WARNING:" \
	"$(TI_SECURE_DEV_PKG)/scripts/create-boot-image.sh not found." \
	"$@ was NOT secured!"; cp $< $@
endif
else
cmd_mkomapsecimg = echo "WARNING: TI_SECURE_DEV_PKG environment" \
	"variable must be defined for TI secure devices. \
	$@ was NOT secured!"; cp $< $@
endif

ifdef CONFIG_SPL_LOAD_FIT
quiet_cmd_omapsecureimg = SECURE  $@
ifneq ($(TI_SECURE_DEV_PKG),)
ifneq ($(wildcard $(TI_SECURE_DEV_PKG)/scripts/secure-binary-image.sh),)
cmd_omapsecureimg = $(TI_SECURE_DEV_PKG)/scripts/secure-binary-image.sh \
	$< $@ \
	$(if $(KBUILD_VERBOSE:1=), >/dev/null)
else
cmd_omapsecureimg = echo "WARNING:" \
	"$(TI_SECURE_DEV_PKG)/scripts/secure-binary-image.sh not found." \
	"$@ was NOT secured!"; cp $< $@
endif
else
cmd_omapsecureimg = echo "WARNING: TI_SECURE_DEV_PKG environment" \
	"variable must be defined for TI secure devices." \
	"$@ was NOT secured!"; cp $< $@
endif
endif


# Standard X-LOADER target (QPSI, NOR flash)
u-boot-spl_HS_X-LOADER: $(obj)/u-boot-spl.bin FORCE
	$(call if_changed,mkomapsecimg)

# For MLO targets (SD card boot) the final file name that is copied to the SD
# card FAT partition must be MLO, so we make a copy of the output file to a new
# file with that name
u-boot-spl_HS_MLO: $(obj)/u-boot-spl.bin FORCE
	$(call if_changed,mkomapsecimg)
	@if [ -f $@ ]; then \
		cp -f $@ MLO; \
	fi

# Standard 2ND target (certain peripheral boot modes)
u-boot-spl_HS_2ND: $(obj)/u-boot-spl.bin FORCE
	$(call if_changed,mkomapsecimg)

# Standard ULO target (certain peripheral boot modes)
u-boot-spl_HS_ULO: $(obj)/u-boot-spl.bin FORCE
	$(call if_changed,mkomapsecimg)

# Standard ISSW target (certain devices, various boot modes), when copied to
# an SD card FAT partition this file must be called "MLO", we make a copy with
# this name to make this clear
u-boot-spl_HS_ISSW: $(obj)/u-boot-spl.bin FORCE
	$(call if_changed,mkomapsecimg)
	@if [ -f $@ ]; then \
		cp -f $@ MLO; \
	fi

# For SPI flash on AM335x and AM43xx, these require special byte swap handling
# so we use the SPI_X-LOADER target instead of X-LOADER and let the
# create-boot-image.sh script handle that
u-boot-spl_HS_SPI_X-LOADER: $(obj)/u-boot-spl.bin FORCE
	$(call if_changed,mkomapsecimg)

# For supporting single stage boot on keystone, the image is a full u-boot
# file, not an SPL. This will work for all boot devices, other than SPI
# flash. On Keystone devices when booting from an SD card FAT partition this
# file must be called "MLO"
u-boot_HS_MLO: $(obj)/u-boot.bin
	$(call if_changed,mkomapsecimg)
	@if [ -f $@ ]; then \
		cp -f $@ MLO; \
	fi

# For supporting single stage XiP QSPI on AM43xx, the image is a full u-boot
# file, not an SPL. In this case the mkomapsecimg command looks for a
# u-boot-HS_* prefix
u-boot_HS_XIP_X-LOADER: $(obj)/u-boot.bin FORCE
	$(call if_changed,mkomapsecimg)

# For supporting the SPL loading and interpreting of FIT images whose
# components are pre-processed before being integrated into the FIT image in
# order to secure them in some way
ifdef CONFIG_SPL_LOAD_FIT

MKIMAGEFLAGS_u-boot_HS.img = -f auto -A $(ARCH) -T firmware -C none -O u-boot \
	-a $(CONFIG_SYS_TEXT_BASE) -e $(CONFIG_SYS_UBOOT_START) \
	-n "U-Boot $(UBOOTRELEASE) for $(BOARD) board" -E \
	$(patsubst %,-b arch/$(ARCH)/dts/%.dtb_HS,$(subst ",,$(CONFIG_OF_LIST)))

OF_LIST_TARGETS = $(patsubst %,arch/$(ARCH)/dts/%.dtb,$(subst ",,$(CONFIG_OF_LIST)))
$(OF_LIST_TARGETS): dtbs

%.dtb_HS: %.dtb FORCE
	$(call if_changed,omapsecureimg)

u-boot-nodtb_HS.bin: u-boot-nodtb.bin FORCE
	$(call if_changed,omapsecureimg)

u-boot_HS.img: u-boot-nodtb_HS.bin u-boot.img $(patsubst %.dtb,%.dtb_HS,$(OF_LIST_TARGETS)) FORCE
	$(call if_changed,mkimage)
	$(Q)if [ -f $@ ]; then \
		cp -f $@ u-boot.img; \
	fi

endif
