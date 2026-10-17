# SPDX-License-Identifier: GPL-2.0+
#
# Copyright 2019 Google LLC

obj-$(CONFIG_SPL_BUILD) += cpu_spl.o
obj-$(CONFIG_SPL_BUILD) += spl.o
obj-$(CONFIG_SPL_BUILD) += systemagent.o
obj-y += cpu_common.o

ifndef CONFIG_TPL_BUILD
obj-y += cpu.o
obj-y += punit.o
obj-y += fsp_bindings.o
ifdef CONFIG_SPL_BUILD
obj-y += fsp_m.o
endif
endif
ifndef CONFIG_SPL_BUILD
obj-y += acpi.o
obj-y += fsp_s.o
endif

obj-y += hostbridge.o
obj-y += lpc.o
obj-y += pch.o
obj-y += pmc.o
obj-y += uart.o
