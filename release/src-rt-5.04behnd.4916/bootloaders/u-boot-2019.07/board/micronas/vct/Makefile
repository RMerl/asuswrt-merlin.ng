# SPDX-License-Identifier: GPL-2.0+
#
# (C) Copyright 2008 Stefan Roese <sr@denx.de>, DENX Software Engineering

obj-y := vct.o
obj-y += ebi.o
obj-$(CONFIG_VCT_NOR) += ebi_nor_flash.o
obj-$(CONFIG_VCT_ONENAND) += ebi_onenand.o
obj-$(CONFIG_DRIVER_SMC911X) += ebi_smc911x.o smc_eeprom.o
obj-y += gpio.o
obj-y += top.o
obj-$(CONFIG_USB_EHCI_VCT) += dcgu.o ehci.o scc.o
