# SPDX-License-Identifier: GPL-2.0+
#
# (C) Copyright 2000-2007
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.

ifneq ($(CONFIG_DM_PCI),)
obj-y += pci_rom.o
obj-$(CONFIG_PCI) += pci-uclass.o pci_auto.o
obj-$(CONFIG_DM_PCI_COMPAT) += pci_compat.o
obj-$(CONFIG_PCI_SANDBOX) += pci_sandbox.o
obj-$(CONFIG_SANDBOX) += pci-emul-uclass.o
obj-$(CONFIG_X86) += pci_x86.o
else
obj-$(CONFIG_PCI) += pci.o pci_auto_old.o
endif
obj-$(CONFIG_PCI) += pci_auto_common.o pci_common.o

obj-$(CONFIG_PCIE_ECAM_GENERIC) += pcie_ecam_generic.o
obj-$(CONFIG_FSL_PCI_INIT) += fsl_pci_init.o
obj-$(CONFIG_PCI_INDIRECT_BRIDGE) += pci_indirect.o
obj-$(CONFIG_PCI_GT64120) += pci_gt64120.o
obj-$(CONFIG_PCI_MSC01) += pci_msc01.o
obj-$(CONFIG_PCIE_IMX) += pcie_imx.o
obj-$(CONFIG_FTPCI100) += pci_ftpci100.o
obj-$(CONFIG_PCI_MVEBU) += pci_mvebu.o
obj-$(CONFIG_PCI_RCAR_GEN2) += pci-rcar-gen2.o
obj-$(CONFIG_PCI_RCAR_GEN3) += pci-rcar-gen3.o
obj-$(CONFIG_SH4_PCI) += pci_sh4.o
obj-$(CONFIG_SH7751_PCI) +=pci_sh7751.o
obj-$(CONFIG_SH7780_PCI) +=pci_sh7780.o
obj-$(CONFIG_PCI_TEGRA) += pci_tegra.o
obj-$(CONFIG_PCI_AARDVARK) += pci-aardvark.o
obj-$(CONFIG_PCIE_DW_MVEBU) += pcie_dw_mvebu.o
obj-$(CONFIG_PCIE_FSL) += pcie_fsl.o pcie_fsl_fixup.o
obj-$(CONFIG_PCIE_LAYERSCAPE) += pcie_layerscape.o
obj-$(CONFIG_PCIE_LAYERSCAPE) += pcie_layerscape_fixup.o
obj-$(CONFIG_PCIE_LAYERSCAPE_GEN4) += pcie_layerscape_gen4.o \
				pcie_layerscape_gen4_fixup.o
obj-$(CONFIG_PCI_XILINX) += pcie_xilinx.o
obj-$(CONFIG_PCIE_INTEL_FPGA) += pcie_intel_fpga.o
