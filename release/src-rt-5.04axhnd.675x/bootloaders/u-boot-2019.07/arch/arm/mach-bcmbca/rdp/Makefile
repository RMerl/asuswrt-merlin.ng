# SPDX-License-Identifier: GPL-2.0+
#
# (C) Copyright 2019 Broadcom Ltd
#

KBUILD_CPPFLAGS += -DLEGACY_RDP -DRDD_BASIC -DBCM_DSL_RDP

ifdef CONFIG_BCM63138
KBUILD_CPPFLAGS += -DDSL_63138
endif

ifdef CONFIG_BCM63148
KBUILD_CPPFLAGS += -DDSL_63148
endif

UBOOTINCLUDE += -I$(srctree)/arch/$(ARCH)/mach-bcmbca/$(subst \",,$(CONFIG_SYS_SOC))/rdp_fw

obj-y +=				\
	unimac_drv_impl1.o		\
	rdp_bbh_arrays.o		\
	rdp_dma_arrays.o		\
	rdp_drv_ih.o			\
	rdp_drv_bpm.o			\
	rdp_drv_sbpm.o			\
	rdp_drv_bbh.o			\
	rdp_cpu_ring.o			\
	rdp_misc.o			\
	rdd_cpu.o			\
	rdd_init.o			\
	rdd_tm.o			\
	rdd_common.o			\
	data_path_init.o		\
