# SPDX-License-Identifier: GPL-2.0+
#
# Copyright (c) 2014 Google, Inc

ifdef CONFIG_HAVE_FSP
obj-y += fsp_configs.o ivybridge.o
else
obj-$(CONFIG_$(SPL_)X86_32BIT_INIT) += cpu.o
obj-y += early_me.o
obj-y += lpc.o
obj-y += northbridge.o
ifndef CONFIG_SPL_BUILD
obj-y += sata.o
endif
obj-$(CONFIG_$(SPL_)X86_32BIT_INIT) += sdram.o
ifndef CONFIG_$(SPL_)X86_32BIT_INIT
obj-y += sdram_nop.o
endif
endif
obj-y += model_206ax.o
obj-y += bd82x6x.o
