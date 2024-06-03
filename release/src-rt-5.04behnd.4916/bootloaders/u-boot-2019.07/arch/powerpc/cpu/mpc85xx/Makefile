# SPDX-License-Identifier: GPL-2.0+
#
# (C) Copyright 2006
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# (C) Copyright 2002,2003 Motorola Inc.
# Xianghua Xiao,X.Xiao@motorola.com

MINIMAL=

ifdef CONFIG_SPL_BUILD
ifdef CONFIG_SPL_INIT_MINIMAL
MINIMAL=y
endif
endif

extra-y = start.o resetvec.o

ifdef MINIMAL

obj-y	+= cpu_init_early.o tlb.o spl_minimal.o

else

obj-$(CONFIG_MP)	+= release.o

ifndef CONFIG_SPL_BUILD
obj-$(CONFIG_CMD_ERRATA) += cmd_errata.o
endif
obj-$(CONFIG_CPM2)	+= commproc.o

obj-$(CONFIG_CPM2)	+= ether_fcc.o
obj-$(CONFIG_OF_LIBFDT) += fdt.o
obj-$(CONFIG_FSL_CORENET) += liodn.o
obj-$(CONFIG_MP)	+= mp.o
obj-$(CONFIG_PCI)	+= pci.o
obj-$(CONFIG_SYS_DPAA_QBMAN) += portals.o

# various SoC specific assignments
obj-$(CONFIG_ARCH_P2041) += p2041_ids.o
obj-$(CONFIG_ARCH_P3041) += p3041_ids.o
obj-$(CONFIG_ARCH_P4080) += p4080_ids.o
obj-$(CONFIG_ARCH_P5020) += p5020_ids.o
obj-$(CONFIG_ARCH_P5040) += p5040_ids.o
obj-$(CONFIG_ARCH_T4240) += t4240_ids.o
obj-$(CONFIG_ARCH_T4160) += t4240_ids.o
obj-$(CONFIG_ARCH_B4420) += b4860_ids.o
obj-$(CONFIG_ARCH_B4860) += b4860_ids.o
obj-$(CONFIG_ARCH_T1040) += t1040_ids.o
obj-$(CONFIG_ARCH_T1042)	+= t1040_ids.o
obj-$(CONFIG_ARCH_T1023) += t1024_ids.o
obj-$(CONFIG_ARCH_T1024) += t1024_ids.o
obj-$(CONFIG_ARCH_T2080) += t2080_ids.o
obj-$(CONFIG_ARCH_T2081) += t2080_ids.o


obj-$(CONFIG_QE)	+= qe_io.o
obj-$(CONFIG_CPM2)	+= serial_scc.o
obj-$(CONFIG_SYS_FSL_QORIQ_CHASSIS1) += fsl_corenet_serdes.o
obj-$(CONFIG_SYS_FSL_QORIQ_CHASSIS2) += fsl_corenet2_serdes.o

# SoC specific SERDES support
obj-$(CONFIG_ARCH_C29X)	+= c29x_serdes.o
obj-$(CONFIG_ARCH_MPC8536) += mpc8536_serdes.o
obj-$(CONFIG_ARCH_MPC8544) += mpc8544_serdes.o
obj-$(CONFIG_ARCH_MPC8548) += mpc8548_serdes.o
obj-$(CONFIG_ARCH_MPC8568) += mpc8568_serdes.o
obj-$(CONFIG_ARCH_MPC8569) += mpc8569_serdes.o
obj-$(CONFIG_ARCH_MPC8572) += mpc8572_serdes.o
obj-$(CONFIG_ARCH_P1010)	+= p1010_serdes.o
obj-$(CONFIG_ARCH_P1011)	+= p1021_serdes.o
obj-$(CONFIG_ARCH_P1020)	+= p1021_serdes.o
obj-$(CONFIG_ARCH_P1021)	+= p1021_serdes.o
obj-$(CONFIG_ARCH_P1022)	+= p1022_serdes.o
obj-$(CONFIG_ARCH_P1023)	+= p1023_serdes.o
obj-$(CONFIG_ARCH_P1024)	+= p1021_serdes.o
obj-$(CONFIG_ARCH_P1025)	+= p1021_serdes.o
obj-$(CONFIG_ARCH_P2020)	+= p2020_serdes.o
obj-$(CONFIG_ARCH_P2041) += p2041_serdes.o
obj-$(CONFIG_ARCH_P3041) += p3041_serdes.o
obj-$(CONFIG_ARCH_P4080) += p4080_serdes.o
obj-$(CONFIG_ARCH_P5020) += p5020_serdes.o
obj-$(CONFIG_ARCH_P5040) += p5040_serdes.o
obj-$(CONFIG_ARCH_T4240) += t4240_serdes.o
obj-$(CONFIG_ARCH_T4160) += t4240_serdes.o
obj-$(CONFIG_ARCH_B4420) += b4860_serdes.o
obj-$(CONFIG_ARCH_B4860) += b4860_serdes.o
obj-$(CONFIG_ARCH_BSC9132) += bsc9132_serdes.o
obj-$(CONFIG_ARCH_T1040) += t1040_serdes.o
obj-$(CONFIG_ARCH_T1042)	+= t1040_serdes.o
obj-$(CONFIG_ARCH_T1023) += t1024_serdes.o
obj-$(CONFIG_ARCH_T1024) += t1024_serdes.o
obj-$(CONFIG_ARCH_T2080) += t2080_serdes.o
obj-$(CONFIG_ARCH_T2081) += t2080_serdes.o

obj-y	+= cpu.o
obj-y	+= cpu_init.o
obj-y	+= cpu_init_early.o
obj-y	+= interrupts.o
ifneq ($(CONFIG_ARCH_QEMU_E500),y)
obj-y	+= speed.o
endif
obj-y	+= tlb.o
obj-y	+= traps.o

endif # not minimal
