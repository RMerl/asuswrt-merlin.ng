/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Based on arch/mips/include/asm/spl.h.
 *
 * (C) Copyright 2012
 * Texas Instruments, <www.ti.com>
 */
#ifndef _ASM_RISCV_SPL_H_
#define _ASM_RISCV_SPL_H_

enum {
	BOOT_DEVICE_RAM,
	BOOT_DEVICE_MMC1,
	BOOT_DEVICE_MMC2,
	BOOT_DEVICE_MMC2_2,
	BOOT_DEVICE_NAND,
	BOOT_DEVICE_ONENAND,
	BOOT_DEVICE_NOR,
	BOOT_DEVICE_UART,
	BOOT_DEVICE_SPI,
	BOOT_DEVICE_USB,
	BOOT_DEVICE_SATA,
	BOOT_DEVICE_NVME,
	BOOT_DEVICE_I2C,
	BOOT_DEVICE_BOARD,
	BOOT_DEVICE_DFU,
	BOOT_DEVICE_XIP,
	BOOT_DEVICE_BOOTROM,
	BOOT_DEVICE_SMH,
	BOOT_DEVICE_NONE
};

/**
 * spl_board_init_f() - initialize board in the SPL phase
 *
 * Return: 0 if succeeded, -ve on error
 */
int spl_board_init_f(void);

#endif
