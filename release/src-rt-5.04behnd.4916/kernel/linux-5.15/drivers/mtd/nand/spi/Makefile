# SPDX-License-Identifier: GPL-2.0
ifdef CONFIG_BCM_KF_MTD_BCMSPINAND
spinand-objs := core.o gigadevice.o macronix.o micron.o paragon.o toshiba.o winbond.o esmt.o etron.o fm.o
else
spinand-objs := core.o gigadevice.o macronix.o micron.o paragon.o toshiba.o winbond.o
endif
obj-$(CONFIG_MTD_SPI_NAND) += spinand.o
