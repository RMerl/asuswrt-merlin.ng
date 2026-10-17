# SPDX-License-Identifier: GPL-2.0+
#
# (C) Copyright 2000-2007
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.

ifdef CONFIG_$(SPL_)DM_USB
obj-y += usb-uclass.o
obj-$(CONFIG_SANDBOX) += usb-sandbox.o
endif

ifdef CONFIG_$(SPL_TPL_)USB_STORAGE
obj-$(CONFIG_$(SPL_TPL_)BOOTSTD) += usb_bootdev.o
endif

# ohci
obj-$(CONFIG_USB_OHCI_NEW) += ohci-hcd.o
obj-$(CONFIG_USB_ATMEL) += ohci-at91.o
obj-$(CONFIG_USB_OHCI_DA8XX) += ohci-da8xx.o
obj-$(CONFIG_USB_R8A66597_HCD) += r8a66597-hcd.o
obj-$(CONFIG_USB_OHCI_LPC32XX) += ohci-lpc32xx.o
obj-$(CONFIG_USB_OHCI_PCI) += ohci-pci.o
obj-$(CONFIG_USB_OHCI_GENERIC) += ohci-generic.o
obj-$(CONFIG_USB_OHCI_BCMBCA) += ohci-bcmbca.o
obj-$(CONFIG_USB_OHCI_NPCM) += ohci-npcm.o

# echi
obj-$(CONFIG_USB_EHCI_HCD) += ehci-hcd.o
obj-$(CONFIG_USB_EHCI_ATMEL) += ehci-atmel.o
obj-$(CONFIG_USB_EHCI_FSL) += ehci-fsl.o
obj-$(CONFIG_USB_EHCI_GENERIC) += ehci-generic.o
obj-$(CONFIG_USB_EHCI_EXYNOS) += ehci-exynos.o
obj-$(CONFIG_USB_EHCI_MXS) += ehci-mxs.o
obj-$(CONFIG_USB_EHCI_MX5) += ehci-mx5.o
obj-$(CONFIG_USB_EHCI_MX6) += ehci-mx6.o
obj-$(CONFIG_USB_EHCI_MX7) += ehci-mx6.o
obj-$(CONFIG_USB_EHCI_NPCM) += ehci-npcm.o
obj-$(CONFIG_USB_EHCI_OMAP) += ehci-omap.o
obj-$(CONFIG_USB_EHCI_MARVELL) += ehci-marvell.o
obj-$(CONFIG_USB_EHCI_MSM) += ehci-msm.o
obj-$(CONFIG_USB_EHCI_PCI) += ehci-pci.o
obj-$(CONFIG_USB_EHCI_TEGRA) += ehci-tegra.o
obj-$(CONFIG_USB_EHCI_VF) += ehci-vf.o
obj-$(CONFIG_USB_EHCI_ZYNQ) += ehci-zynq.o
obj-$(CONFIG_USB_EHCI_BCMBCA) += ehci-bcmbca.o

# xhci
obj-$(CONFIG_USB_XHCI_BRCM) += xhci-brcm.o
obj-$(CONFIG_USB_XHCI_HCD) += xhci.o xhci-mem.o xhci-ring.o
obj-$(CONFIG_USB_XHCI_DWC3) += xhci-dwc3.o
obj-$(CONFIG_USB_XHCI_DWC3_OF_SIMPLE) += dwc3-of-simple.o
obj-$(CONFIG_USB_XHCI_EXYNOS) += xhci-exynos5.o
obj-$(CONFIG_USB_XHCI_FSL) += xhci-fsl.o
obj-$(CONFIG_USB_XHCI_MTK) += xhci-mtk.o
obj-$(CONFIG_USB_XHCI_MVEBU) += xhci-mvebu.o
obj-$(CONFIG_USB_XHCI_OMAP) += xhci-omap.o
obj-$(CONFIG_USB_XHCI_PCI) += xhci-pci.o
obj-$(CONFIG_USB_XHCI_RCAR) += xhci-rcar.o
obj-$(CONFIG_USB_XHCI_STI) += dwc3-sti-glue.o
obj-$(CONFIG_USB_XHCI_OCTEON) += dwc3-octeon-glue.o

# designware
obj-$(CONFIG_USB_DWC2) += dwc2.o
