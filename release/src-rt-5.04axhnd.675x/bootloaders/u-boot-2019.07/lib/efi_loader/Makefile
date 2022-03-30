# SPDX-License-Identifier: GPL-2.0+
#
# (C) Copyright 2016 Alexander Graf
#

# This file only gets included with CONFIG_EFI_LOADER set, so all
# object inclusion implicitly depends on it

CFLAGS_efi_boottime.o += \
  -DFW_VERSION="0x$(VERSION)" \
  -DFW_PATCHLEVEL="0x$(PATCHLEVEL)"
CFLAGS_helloworld.o := $(CFLAGS_EFI) -Os -ffreestanding
CFLAGS_REMOVE_helloworld.o := $(CFLAGS_NON_EFI)

ifneq ($(CONFIG_CMD_BOOTEFI_HELLO_COMPILE),)
always += helloworld.efi
endif

obj-$(CONFIG_CMD_BOOTEFI_HELLO) += helloworld_efi.o
obj-y += efi_bootmgr.o
obj-y += efi_boottime.o
obj-y += efi_console.o
obj-y += efi_device_path.o
obj-$(CONFIG_EFI_DEVICE_PATH_TO_TEXT) += efi_device_path_to_text.o
obj-y += efi_device_path_utilities.o
obj-y += efi_file.o
obj-$(CONFIG_EFI_LOADER_HII) += efi_hii.o efi_hii_config.o
obj-y += efi_image_loader.o
obj-y += efi_memory.o
obj-y += efi_root_node.o
obj-y += efi_runtime.o
obj-y += efi_setup.o
obj-$(CONFIG_EFI_UNICODE_COLLATION_PROTOCOL2) += efi_unicode_collation.o
obj-y += efi_variable.o
obj-y += efi_watchdog.o
obj-$(CONFIG_LCD) += efi_gop.o
obj-$(CONFIG_DM_VIDEO) += efi_gop.o
obj-$(CONFIG_PARTITIONS) += efi_disk.o
obj-$(CONFIG_NET) += efi_net.o
obj-$(CONFIG_GENERATE_ACPI_TABLE) += efi_acpi.o
obj-$(CONFIG_GENERATE_SMBIOS_TABLE) += efi_smbios.o
