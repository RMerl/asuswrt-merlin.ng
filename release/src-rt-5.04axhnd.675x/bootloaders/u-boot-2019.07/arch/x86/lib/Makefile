# SPDX-License-Identifier: GPL-2.0+
#
# (C) Copyright 2002-2006
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.

ifndef CONFIG_X86_64
obj-y += bios.o
obj-y += bios_asm.o
obj-y += bios_interrupts.o
obj-y += string.o
endif
ifndef CONFIG_SPL_BUILD
obj-$(CONFIG_CMD_BOOTM) += bootm.o
endif
obj-y	+= cmd_boot.o
obj-$(CONFIG_SEABIOS) += coreboot_table.o
obj-y	+= early_cmos.o
obj-y	+= e820.o
obj-y	+= init_helpers.o
obj-y	+= interrupts.o
obj-y	+= lpc-uclass.o
obj-y	+= mpspec.o
obj-$(CONFIG_ENABLE_MRC_CACHE) += mrccache.o
obj-y	+= northbridge-uclass.o
obj-$(CONFIG_I8259_PIC) += i8259.o
obj-$(CONFIG_I8254_TIMER) += i8254.o
obj-$(CONFIG_PINCTRL_ICH6) += pinctrl_ich6.o
obj-y	+= pirq_routing.o
obj-y	+= relocate.o
obj-y += physmem.o
obj-$(CONFIG_INTEL_MID) += pmu.o
obj-$(CONFIG_X86_RAMTEST) += ramtest.o
obj-$(CONFIG_INTEL_MID) += scu.o
obj-y	+= sections.o
obj-y += sfi.o
obj-y	+= acpi.o
obj-$(CONFIG_HAVE_ACPI_RESUME) += acpi_s3.o
ifndef CONFIG_QEMU
obj-$(CONFIG_GENERATE_ACPI_TABLE) += acpi_table.o
endif
obj-y	+= tables.o
ifndef CONFIG_SPL_BUILD
obj-$(CONFIG_CMD_ZBOOT)	+= zimage.o
endif
obj-$(CONFIG_HAVE_FSP) += fsp/

ifdef CONFIG_SPL_BUILD
ifdef CONFIG_TPL_BUILD
obj-y += tpl.o
else
obj-y += spl.o
endif
endif

lib-$(CONFIG_USE_PRIVATE_LIBGCC) += div64.o

ifeq ($(CONFIG_$(SPL_)X86_64),)
obj-$(CONFIG_EFI_APP) += crt0_ia32_efi.o reloc_ia32_efi.o
endif

ifneq ($(CONFIG_EFI_STUB),)

CFLAGS_REMOVE_reloc_ia32_efi.o += -mregparm=3
CFLAGS_reloc_ia32_efi.o += -fpic -fshort-wchar

# When building for 64-bit we must remove the i386-specific flags
CFLAGS_REMOVE_reloc_x86_64_efi.o += -mregparm=3 -march=i386 -m32
CFLAGS_reloc_x86_64_efi.o += -fpic -fshort-wchar -m64

AFLAGS_REMOVE_crt0_x86_64_efi.o += -mregparm=3 -march=i386 -m32
AFLAGS_crt0_x86_64_efi.o += -fpic -fshort-wchar -m64

extra-$(CONFIG_EFI_STUB_32BIT) += crt0_ia32_efi.o reloc_ia32_efi.o
extra-$(CONFIG_EFI_STUB_64BIT) += crt0_x86_64_efi.o reloc_x86_64_efi.o

endif

ifdef CONFIG_EFI_STUB

ifeq ($(CONFIG_$(SPL_)X86_64),)
extra-y += $(EFI_CRT0) $(EFI_RELOC)
endif

else

ifndef CONFIG_SPL_BUILD
ifneq ($(CONFIG_CMD_BOOTEFI_SELFTEST)$(CONFIG_CMD_BOOTEFI_HELLO_COMPILE),)
extra-y += $(EFI_CRT0) $(EFI_RELOC)
endif
endif

endif
