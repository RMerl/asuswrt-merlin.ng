# SPDX-License-Identifier: GPL-2.0+
#
# (C) Copyright 2017, Heinrich Schuchardt <xypron.glpk@gmx.de>

# This file only gets included with CONFIG_EFI_LOADER set, so all
# object inclusion implicitly depends on it

asflags-y += -DHOST_ARCH="$(HOST_ARCH)"
ccflags-y += -DHOST_ARCH="$(HOST_ARCH)"

CFLAGS_efi_selftest_miniapp_exception.o := $(CFLAGS_EFI) -Os -ffreestanding
CFLAGS_REMOVE_efi_selftest_miniapp_exception.o := $(CFLAGS_NON_EFI)
CFLAGS_efi_selftest_miniapp_exit.o := $(CFLAGS_EFI) -Os -ffreestanding
CFLAGS_REMOVE_efi_selftest_miniapp_exit.o := $(CFLAGS_NON_EFI)
CFLAGS_efi_selftest_miniapp_return.o := $(CFLAGS_EFI) -Os -ffreestanding
CFLAGS_REMOVE_efi_selftest_miniapp_return.o := $(CFLAGS_NON_EFI)

obj-y += \
efi_selftest.o \
efi_selftest_bitblt.o \
efi_selftest_config_table.o \
efi_selftest_controllers.o \
efi_selftest_console.o \
efi_selftest_crc32.o \
efi_selftest_devicepath_util.o \
efi_selftest_events.o \
efi_selftest_event_groups.o \
efi_selftest_exception.o \
efi_selftest_exitbootservices.o \
efi_selftest_gop.o \
efi_selftest_load_file.o \
efi_selftest_loaded_image.o \
efi_selftest_loadimage.o \
efi_selftest_manageprotocols.o \
efi_selftest_mem.o \
efi_selftest_memory.o \
efi_selftest_open_protocol.o \
efi_selftest_register_notify.o \
efi_selftest_reset.o \
efi_selftest_set_virtual_address_map.o \
efi_selftest_startimage_exit.o \
efi_selftest_startimage_return.o \
efi_selftest_textinput.o \
efi_selftest_textinputex.o \
efi_selftest_textoutput.o \
efi_selftest_tpl.o \
efi_selftest_util.o \
efi_selftest_variables.o \
efi_selftest_variables_runtime.o \
efi_selftest_watchdog.o

obj-$(CONFIG_EFI_ECPT) += efi_selftest_ecpt.o
obj-$(CONFIG_NETDEVICES) += efi_selftest_snp.o

obj-$(CONFIG_EFI_DEVICE_PATH_TO_TEXT) += efi_selftest_devicepath.o
obj-$(CONFIG_EFI_UNICODE_COLLATION_PROTOCOL2) += \
efi_selftest_unicode_collation.o

ifeq ($(CONFIG_CPU_V7A)$(CONFIG_CPU_V7M)$(CONFIG_CPU_V7R),y)
obj-y += efi_selftest_unaligned.o
endif
obj-$(CONFIG_EFI_LOADER_HII) += efi_selftest_hii.o
obj-$(CONFIG_EFI_RNG_PROTOCOL) += efi_selftest_rng.o
obj-$(CONFIG_EFI_GET_TIME) += efi_selftest_rtc.o
obj-$(CONFIG_EFI_TCG2_PROTOCOL) += efi_selftest_tcg2.o

ifeq ($(CONFIG_GENERATE_ACPI_TABLE),)
obj-y += efi_selftest_fdt.o
endif

ifeq ($(CONFIG_BLK)$(CONFIG_DOS_PARTITION),yy)
obj-y += efi_selftest_block_device.o
endif

obj-$(CONFIG_EFI_ESRT) += efi_selftest_esrt.o

targets += \
efi_miniapp_file_image_exception.h \
efi_miniapp_file_image_exit.h \
efi_miniapp_file_image_return.h \
efi_selftest_miniapp_exception.efi \
efi_selftest_miniapp_exit.efi \
efi_selftest_miniapp_return.efi

$(obj)/efi_miniapp_file_image_exception.h: $(obj)/efi_selftest_miniapp_exception.efi
	$(obj)/../../tools/file2include $(obj)/efi_selftest_miniapp_exception.efi > \
	$(obj)/efi_miniapp_file_image_exception.h

$(obj)/efi_miniapp_file_image_exit.h: $(obj)/efi_selftest_miniapp_exit.efi
	$(obj)/../../tools/file2include $(obj)/efi_selftest_miniapp_exit.efi > \
	$(obj)/efi_miniapp_file_image_exit.h

$(obj)/efi_miniapp_file_image_return.h: $(obj)/efi_selftest_miniapp_return.efi
	$(obj)/../../tools/file2include $(obj)/efi_selftest_miniapp_return.efi > \
	$(obj)/efi_miniapp_file_image_return.h

$(obj)/efi_selftest_exception.o: $(obj)/efi_miniapp_file_image_exception.h

$(obj)/efi_selftest_load_file.o: $(obj)/efi_miniapp_file_image_exit.h

$(obj)/efi_selftest_loadimage.o: $(obj)/efi_miniapp_file_image_exit.h

$(obj)/efi_selftest_startimage_exit.o: $(obj)/efi_miniapp_file_image_exit.h

$(obj)/efi_selftest_startimage_return.o: $(obj)/efi_miniapp_file_image_return.h
