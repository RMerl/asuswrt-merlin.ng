# SPDX-License-Identifier: GPL-2.0+
#
# (C) Copyright 2015 Google, Inc

obj-$(CONFIG_EFI_APP) += efi_app.o efi.o
obj-$(CONFIG_EFI_STUB) += efi_info.o

CFLAGS_REMOVE_efi_stub.o := -mregparm=3 \
	$(if $(CONFIG_EFI_STUB_64BIT),-march=i386 -m32)
CFLAGS_efi_stub.o := -fpic -fshort-wchar \
	$(if $(CONFIG_EFI_STUB_64BIT),-m64)
CFLAGS_REMOVE_efi.o := -mregparm=3 \
	$(if $(CONFIG_EFI_STUB_64BIT),-march=i386 -m32)
CFLAGS_efi.o := -fpic -fshort-wchar \
	$(if $(CONFIG_EFI_STUB_64BIT),-m64)

extra-$(CONFIG_EFI_STUB) += efi_stub.o efi.o
