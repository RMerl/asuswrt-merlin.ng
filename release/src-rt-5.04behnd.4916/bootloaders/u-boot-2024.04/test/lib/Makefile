# SPDX-License-Identifier: GPL-2.0+
#
# (C) Copyright 2018
# Mario Six, Guntermann & Drunck GmbH, mario.six@gdsys.cc
ifeq ($(CONFIG_SPL_BUILD),)
obj-y += cmd_ut_lib.o
obj-y += abuf.o
obj-$(CONFIG_EFI_LOADER) += efi_device_path.o
obj-$(CONFIG_EFI_SECURE_BOOT) += efi_image_region.o
obj-y += hexdump.o
obj-$(CONFIG_SANDBOX) += kconfig.o
obj-y += lmb.o
obj-y += longjmp.o
obj-$(CONFIG_CONSOLE_RECORD) += test_print.o
obj-$(CONFIG_SSCANF) += sscanf.o
obj-y += string.o
obj-y += strlcat.o
obj-$(CONFIG_ERRNO_STR) += test_errno_str.o
obj-$(CONFIG_UT_LIB_ASN1) += asn1.o
obj-$(CONFIG_UT_LIB_RSA) += rsa.o
obj-$(CONFIG_AES) += test_aes.o
obj-$(CONFIG_GETOPT) += getopt.o
obj-$(CONFIG_CRC8) += test_crc8.o
obj-$(CONFIG_UT_LIB_CRYPT) += test_crypt.o
obj-$(CONFIG_LIB_UUID) += uuid.o
else
obj-$(CONFIG_SANDBOX) += kconfig_spl.o
endif
