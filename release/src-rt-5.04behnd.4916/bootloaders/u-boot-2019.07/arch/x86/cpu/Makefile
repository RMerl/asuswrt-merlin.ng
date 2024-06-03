# SPDX-License-Identifier: GPL-2.0+
#
# (C) Copyright 2006
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# (C) Copyright 2002
# Daniel Engström, Omicron Ceti AB, daniel@omicron.se.

ifeq ($(CONFIG_$(SPL_)X86_64),y)
extra-y	= start64.o
else
ifeq ($(CONFIG_$(SPL_TPL_)X86_16BIT_INIT),y)
extra-y	= start.o
else
ifndef CONFIG_SPL
extra-y	= start.o
else
ifdef CONFIG_SPL_BUILD
extra-y	= start_from_tpl.o
else
extra-y	= start_from_spl.o
endif
endif
endif
endif

extra-$(CONFIG_$(SPL_TPL_)X86_16BIT_INIT) += resetvec.o start16.o

obj-y	+= cpu.o cpu_x86.o

ifndef CONFIG_$(SPL_)X86_64
AFLAGS_REMOVE_call32.o := -mregparm=3 \
	$(if $(CONFIG_EFI_STUB_64BIT),-march=i386 -m32)
AFLAGS_call32.o := -fpic -fshort-wchar \
	$(if $(CONFIG_EFI_STUB_64BIT),-m64)

extra-y += call32.o
endif

obj-y += intel_common/
obj-$(CONFIG_INTEL_BAYTRAIL) += baytrail/
obj-$(CONFIG_INTEL_BRASWELL) += braswell/
obj-$(CONFIG_INTEL_BROADWELL) += broadwell/
obj-$(CONFIG_SYS_COREBOOT) += coreboot/
obj-$(CONFIG_EFI) += efi/
obj-$(CONFIG_QEMU) += qemu/
obj-$(CONFIG_NORTHBRIDGE_INTEL_IVYBRIDGE) += ivybridge/
obj-$(CONFIG_INTEL_QUARK) += quark/
obj-$(CONFIG_INTEL_QUEENSBAY) += queensbay/
obj-$(CONFIG_INTEL_TANGIER) += tangier/
obj-$(CONFIG_APIC) += lapic.o ioapic.o
obj-y += irq.o
ifndef CONFIG_$(SPL_)X86_64
obj-$(CONFIG_SMP) += mp_init.o
endif
obj-y += mtrr.o
obj-$(CONFIG_PCI) += pci.o
ifndef CONFIG_$(SPL_)X86_64
obj-$(CONFIG_SMP) += sipi_vector.o
endif
obj-y += turbo.o
obj-$(CONFIG_HAVE_ACPI_RESUME) += wakeup.o

ifeq ($(CONFIG_$(SPL_)X86_64),y)
obj-y += x86_64/
else
obj-y += i386/
endif
