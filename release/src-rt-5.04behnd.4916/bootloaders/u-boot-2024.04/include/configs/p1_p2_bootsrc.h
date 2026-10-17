/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2010-2011 Freescale Semiconductor, Inc.
 * Copyright 2020 NXP
 * Copyright 2022 Pali Rohár <pali@kernel.org>
 */

#include <linux/stringify.h>

#if !defined(CONFIG_SYS_SPD_BUS_NUM) || !defined(CFG_SYS_I2C_PCA9557_ADDR)
#error "CONFIG_SYS_SPD_BUS_NUM and CFG_SYS_I2C_PCA9557_ADDR are required"
#endif

#define __BOOTSRC_CMD(src, msk) i2c dev CONFIG_SYS_SPD_BUS_NUM; i2c mw CFG_SYS_I2C_PCA9557_ADDR 1 src 1; i2c mw CFG_SYS_I2C_PCA9557_ADDR 3 msk 1

#define __VAR_CMD(var, cmd) __stringify(var=cmd\0)
#define __VAR_CMD_RST(var, cmd) __VAR_CMD(var, cmd; reset)

#ifdef __SW_NOR_BANK_LO
#define MAP_NOR_LO_CMD(var, ...) __VAR_CMD(var, __VA_ARGS__ __BOOTSRC_CMD(__SW_NOR_BANK_LO, __SW_NOR_BANK_MASK))
#else
#define MAP_NOR_LO_CMD(var, ...) ""
#endif

#ifdef __SW_NOR_BANK_UP
#define MAP_NOR_UP_CMD(var, ...) __VAR_CMD(var, __VA_ARGS__ __BOOTSRC_CMD(__SW_NOR_BANK_UP, __SW_NOR_BANK_MASK))
#else
#define MAP_NOR_UP_CMD(var, ...) ""
#endif

#ifdef __SW_BOOT_NOR
#define RST_NOR_CMD(var, ...) __VAR_CMD_RST(var, __VA_ARGS__ __BOOTSRC_CMD(__SW_BOOT_NOR, __SW_BOOT_MASK))
#else
#define RST_NOR_CMD(var, ...) ""
#endif

#ifdef __SW_BOOT_NOR_BANK_LO
#define RST_NOR_LO_CMD(var, ...) __VAR_CMD_RST(var, __VA_ARGS__ __BOOTSRC_CMD(__SW_BOOT_NOR_BANK_LO, __SW_BOOT_MASK))
#else
#define RST_NOR_LO_CMD(var, ...) ""
#endif

#ifdef __SW_BOOT_NOR_BANK_UP
#define RST_NOR_UP_CMD(var, ...) __VAR_CMD_RST(var, __VA_ARGS__ __BOOTSRC_CMD(__SW_BOOT_NOR_BANK_UP, __SW_BOOT_MASK))
#else
#define RST_NOR_UP_CMD(var, ...) ""
#endif

#ifdef __SW_BOOT_SPI
#define RST_SPI_CMD(var, ...) __VAR_CMD_RST(var, __VA_ARGS__ __BOOTSRC_CMD(__SW_BOOT_SPI, __SW_BOOT_MASK))
#else
#define RST_SPI_CMD(var, ...) ""
#endif

#ifdef __SW_BOOT_SD
#define RST_SD_CMD(var, ...) __VAR_CMD_RST(var, __VA_ARGS__ __BOOTSRC_CMD(__SW_BOOT_SD, __SW_BOOT_MASK))
#else
#define RST_SD_CMD(var, ...) ""
#endif

#ifdef __SW_BOOT_SD2
#define RST_SD2_CMD(var, ...) __VAR_CMD_RST(var, __VA_ARGS__ __BOOTSRC_CMD(__SW_BOOT_SD2, __SW_BOOT_MASK))
#else
#define RST_SD2_CMD(var, ...) ""
#endif

#ifdef __SW_BOOT_NAND
#define RST_NAND_CMD(var, ...) __VAR_CMD_RST(var, __VA_ARGS__ __BOOTSRC_CMD(__SW_BOOT_NAND, __SW_BOOT_MASK))
#else
#define RST_NAND_CMD(var, ...) ""
#endif

#ifdef __SW_BOOT_PCIE
#define RST_PCIE_CMD(var, ...) __VAR_CMD_RST(var, __VA_ARGS__ __BOOTSRC_CMD(__SW_BOOT_PCIE, __SW_BOOT_MASK))
#else
#define RST_PCIE_CMD(var, ...) ""
#endif

#define RST_DEF_CMD(var, ...) __VAR_CMD_RST(var, __VA_ARGS__ __BOOTSRC_CMD(0x00, 0xff))
