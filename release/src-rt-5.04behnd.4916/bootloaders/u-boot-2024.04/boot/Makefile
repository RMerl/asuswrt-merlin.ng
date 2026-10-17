# SPDX-License-Identifier: GPL-2.0+
#
# (C) Copyright 2004-2006
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.

ifndef CONFIG_SPL_BUILD

obj-$(CONFIG_BOOT_RETRY) += bootretry.o
obj-$(CONFIG_CMD_BOOTM) += bootm.o bootm_os.o
obj-$(CONFIG_CMD_BOOTZ) += bootm.o bootm_os.o
obj-$(CONFIG_CMD_BOOTI) += bootm.o bootm_os.o

obj-$(CONFIG_PXE_UTILS) += pxe_utils.o
obj-$(CONFIG_QFW) += bootmeth_qfw.o

endif

obj-y += image.o image-board.o

obj-$(CONFIG_ANDROID_AB) += android_ab.o
obj-$(CONFIG_ANDROID_BOOT_IMAGE) += image-android.o image-android-dt.o

obj-$(CONFIG_$(SPL_TPL_)BOOTSTD) += bootdev-uclass.o
obj-$(CONFIG_$(SPL_TPL_)BOOTSTD) += bootflow.o
obj-$(CONFIG_$(SPL_TPL_)BOOTSTD) += bootmeth-uclass.o
obj-$(CONFIG_$(SPL_TPL_)BOOTSTD) += bootstd-uclass.o

obj-$(CONFIG_$(SPL_TPL_)BOOTSTD_PROG) += prog_boot.o

obj-$(CONFIG_$(SPL_TPL_)BOOTMETH_EXTLINUX) += bootmeth_extlinux.o
obj-$(CONFIG_$(SPL_TPL_)BOOTMETH_EXTLINUX_PXE) += bootmeth_pxe.o
obj-$(CONFIG_$(SPL_TPL_)BOOTMETH_EFILOADER) += bootmeth_efi.o
obj-$(CONFIG_$(SPL_TPL_)BOOTMETH_CROS) += bootm.o bootm_os.o bootmeth_cros.o
obj-$(CONFIG_$(SPL_TPL_)BOOTMETH_SANDBOX) += bootmeth_sandbox.o
obj-$(CONFIG_$(SPL_TPL_)BOOTMETH_SCRIPT) += bootmeth_script.o
obj-$(CONFIG_$(SPL_TPL_)CEDIT) += cedit.o
ifdef CONFIG_$(SPL_TPL_)BOOTSTD_FULL
obj-$(CONFIG_EFI_BOOTMGR) += bootmeth_efi_mgr.o
obj-$(CONFIG_$(SPL_TPL_)EXPO) += bootflow_menu.o
obj-$(CONFIG_$(SPL_TPL_)BOOTSTD) += bootflow_menu.o
endif

obj-$(CONFIG_$(SPL_TPL_)OF_LIBFDT) += fdt_support.o
obj-$(CONFIG_$(SPL_TPL_)FDT_SIMPLEFB) += fdt_simplefb.o

obj-$(CONFIG_$(SPL_TPL_)OF_LIBFDT) += image-fdt.o
obj-$(CONFIG_$(SPL_TPL_)FIT_SIGNATURE) += fdt_region.o
obj-$(CONFIG_$(SPL_TPL_)FIT) += image-fit.o
obj-$(CONFIG_$(SPL_)MULTI_DTB_FIT) += boot_fit.o common_fit.o
obj-$(CONFIG_$(SPL_TPL_)IMAGE_PRE_LOAD) += image-pre-load.o
obj-$(CONFIG_$(SPL_TPL_)IMAGE_SIGN_INFO) += image-sig.o
obj-$(CONFIG_$(SPL_TPL_)FIT_SIGNATURE) += image-fit-sig.o
obj-$(CONFIG_$(SPL_TPL_)FIT_CIPHER) += image-cipher.o

obj-$(CONFIG_CMD_ADTIMG) += image-android-dt.o

ifdef CONFIG_SPL_BUILD
obj-$(CONFIG_$(SPL_TPL_)LOAD_FIT) += common_fit.o
endif

obj-$(CONFIG_$(SPL_TPL_)EXPO) += expo.o scene.o expo_build.o
obj-$(CONFIG_$(SPL_TPL_)EXPO) += scene_menu.o scene_textline.o

obj-$(CONFIG_$(SPL_TPL_)BOOTMETH_VBE) += vbe.o
obj-$(CONFIG_$(SPL_TPL_)BOOTMETH_VBE_REQUEST) += vbe_request.o
obj-$(CONFIG_$(SPL_TPL_)BOOTMETH_VBE_SIMPLE) += vbe_simple.o
obj-$(CONFIG_$(SPL_TPL_)BOOTMETH_VBE_SIMPLE_FW) += vbe_simple_fw.o
obj-$(CONFIG_$(SPL_TPL_)BOOTMETH_VBE_SIMPLE_OS) += vbe_simple_os.o
