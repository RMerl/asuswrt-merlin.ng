# SPDX-License-Identifier: GPL-2.0+
#
# (C) Copyright 2016 Alexander Graf
#

# This file only gets included with CONFIG_EFI_LOADER set, so all
# object inclusion implicitly depends on it

asflags-y += -DHOST_ARCH="$(HOST_ARCH)" -I.
ccflags-y += -DHOST_ARCH="$(HOST_ARCH)"

CFLAGS_efi_boottime.o += \
  -DFW_VERSION="0x$(VERSION)" \
  -DFW_PATCHLEVEL="0x$(PATCHLEVEL)"
CFLAGS_boothart.o := $(CFLAGS_EFI) -Os -ffreestanding
CFLAGS_REMOVE_boothart.o := $(CFLAGS_NON_EFI)
CFLAGS_helloworld.o := $(CFLAGS_EFI) -Os -ffreestanding
CFLAGS_REMOVE_helloworld.o := $(CFLAGS_NON_EFI)
CFLAGS_smbiosdump.o := $(CFLAGS_EFI) -Os -ffreestanding
CFLAGS_REMOVE_smbiosdump.o := $(CFLAGS_NON_EFI)
CFLAGS_dtbdump.o := $(CFLAGS_EFI) -Os -ffreestanding
CFLAGS_REMOVE_dtbdump.o := $(CFLAGS_NON_EFI)
CFLAGS_initrddump.o := $(CFLAGS_EFI) -Os -ffreestanding
CFLAGS_REMOVE_initrddump.o := $(CFLAGS_NON_EFI)

ifdef CONFIG_RISCV
always += boothart.efi
targets += boothart.o
endif

ifneq ($(CONFIG_CMD_BOOTEFI_HELLO_COMPILE),)
always += helloworld.efi
targets += helloworld.o
endif

ifneq ($(CONFIG_GENERATE_SMBIOS_TABLE),)
always += smbiosdump.efi
targets += smbiosdump.o
endif

ifeq ($(CONFIG_GENERATE_ACPI_TABLE),)
always += dtbdump.efi
targets += dtbdump.o
endif

ifdef CONFIG_EFI_LOAD_FILE2_INITRD
always += initrddump.efi
targets += initrddump.o
endif

obj-$(CONFIG_CMD_BOOTEFI_HELLO) += helloworld_efi.o
obj-$(CONFIG_EFI_BOOTMGR) += efi_bootmgr.o
obj-$(CONFIG_EFI_BINARY_EXEC) += efi_bootbin.o
obj-y += efi_boottime.o
obj-y += efi_helper.o
obj-$(CONFIG_EFI_HAVE_CAPSULE_SUPPORT) += efi_capsule.o
obj-$(CONFIG_EFI_CAPSULE_FIRMWARE) += efi_firmware.o
obj-y += efi_console.o
obj-y += efi_device_path.o
obj-$(CONFIG_EFI_DEVICE_PATH_TO_TEXT) += efi_device_path_to_text.o
obj-$(CONFIG_EFI_DEVICE_PATH_UTIL) += efi_device_path_utilities.o
obj-y += efi_dt_fixup.o
obj-y += efi_file.o
obj-$(CONFIG_EFI_LOADER_HII) += efi_hii.o
obj-y += efi_image_loader.o
obj-y += efi_load_options.o
obj-y += efi_memory.o
obj-y += efi_root_node.o
obj-y += efi_runtime.o
obj-y += efi_setup.o
obj-y += efi_string.o
obj-$(CONFIG_EFI_UNICODE_COLLATION_PROTOCOL2) += efi_unicode_collation.o
obj-y += efi_var_common.o
obj-y += efi_var_mem.o
obj-y += efi_var_file.o
ifeq ($(CONFIG_EFI_MM_COMM_TEE),y)
obj-y += efi_variable_tee.o
else
obj-y += efi_variable.o
obj-$(CONFIG_EFI_VARIABLES_PRESEED) += efi_var_seed.o
endif
obj-y += efi_watchdog.o
obj-$(CONFIG_EFI_ESRT) += efi_esrt.o
obj-$(CONFIG_VIDEO) += efi_gop.o
obj-$(CONFIG_BLK) += efi_disk.o
obj-$(CONFIG_NETDEVICES) += efi_net.o
obj-$(CONFIG_ACPI) += efi_acpi.o
obj-$(CONFIG_SMBIOS) += efi_smbios.o
obj-$(CONFIG_EFI_RNG_PROTOCOL) += efi_rng.o
obj-$(CONFIG_EFI_TCG2_PROTOCOL) += efi_tcg2.o
obj-$(CONFIG_EFI_RISCV_BOOT_PROTOCOL) += efi_riscv.o
obj-$(CONFIG_EFI_LOAD_FILE2_INITRD) += efi_load_initrd.o
obj-$(CONFIG_EFI_SIGNATURE_SUPPORT) += efi_signature.o
obj-$(CONFIG_EFI_ECPT) += efi_conformance.o

EFI_VAR_SEED_FILE := $(subst $\",,$(CONFIG_EFI_VAR_SEED_FILE))
$(obj)/efi_var_seed.o: $(srctree)/$(EFI_VAR_SEED_FILE)
