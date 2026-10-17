# SPDX-License-Identifier: GPL-2.0+
#
# (C) Copyright 2008
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.

obj-y += fpga.o
obj-$(CONFIG_DM_FPGA) += fpga-uclass.o
obj-$(CONFIG_SANDBOX_FPGA) += sandbox.o

obj-$(CONFIG_FPGA_SPARTAN2) += spartan2.o
obj-$(CONFIG_FPGA_SPARTAN3) += spartan3.o
obj-$(CONFIG_FPGA_VERSALPL) += versalpl.o
obj-$(CONFIG_FPGA_VIRTEX2) += virtex2.o
obj-$(CONFIG_FPGA_ZYNQPL) += zynqpl.o
obj-$(CONFIG_FPGA_ZYNQMPPL) += zynqmppl.o
obj-$(CONFIG_FPGA_XILINX) += xilinx.o
obj-$(CONFIG_FPGA_LATTICE) += ivm_core.o lattice.o
ifdef CONFIG_FPGA_ALTERA
obj-y += altera.o
obj-$(CONFIG_FPGA_ACEX1K) += ACEX1K.o
obj-$(CONFIG_FPGA_CYCLON2) += cyclon2.o
obj-$(CONFIG_FPGA_INTEL_SDM_MAILBOX) += intel_sdm_mb.o
obj-$(CONFIG_FPGA_STRATIX_II) += stratixII.o
obj-$(CONFIG_FPGA_STRATIX_V) += stratixv.o
obj-$(CONFIG_FPGA_SOCFPGA) += socfpga.o
obj-$(CONFIG_TARGET_SOCFPGA_GEN5) += socfpga_gen5.o
obj-$(CONFIG_TARGET_SOCFPGA_ARRIA10) += socfpga_arria10.o
endif
