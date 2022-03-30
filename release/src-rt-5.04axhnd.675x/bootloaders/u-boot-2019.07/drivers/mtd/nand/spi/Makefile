# SPDX-License-Identifier: GPL-2.0

ifndef CONFIG_SPL_BUILD
spinand-objs := core.o gigadevice.o macronix.o micron.o winbond.o broadcom.o
obj-$(CONFIG_MTD_SPI_NAND) += spinand.o
else
obj-$(CONFIG_MTD_SPI_NAND) += spinand_mini.o
endif
