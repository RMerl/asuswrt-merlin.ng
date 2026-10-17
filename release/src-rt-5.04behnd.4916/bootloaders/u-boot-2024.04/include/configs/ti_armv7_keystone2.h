/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Common configuration header file for all Keystone II EVM platforms
 *
 * (C) Copyright 2012-2014
 *     Texas Instruments Incorporated, <www.ti.com>
 */

#ifndef __CONFIG_KS2_EVM_H
#define __CONFIG_KS2_EVM_H

/* Memory Configuration */
#define CFG_SYS_LPAE_SDRAM_BASE	0x800000000
#define CFG_MAX_RAM_BANK_SIZE	(2 << 30)       /* 2GB */

/* SRAM scratch space entries  */
#define SRAM_SCRATCH_SPACE_ADDR		0xc0c23fc

#define TI_SRAM_SCRATCH_BOARD_EEPROM_START	(SRAM_SCRATCH_SPACE_ADDR)
#define TI_SRAM_SCRATCH_BOARD_EEPROM_END	(SRAM_SCRATCH_SPACE_ADDR + 0x200)
#define KEYSTONE_SRAM_SCRATCH_SPACE_END		(TI_SRAM_SCRATCH_BOARD_EEPROM_END)

/* UART Configuration */
#define CFG_SYS_NS16550_COM1		KS2_UART0_BASE
#define CFG_SYS_NS16550_COM2		KS2_UART1_BASE

#ifndef CONFIG_SOC_K2G
#define CFG_SYS_NS16550_CLK		ks_clk_get_rate(KS2_CLK1_6)
#else
#define CFG_SYS_NS16550_CLK		ks_clk_get_rate(uart_pll_clk) / 2
#endif

/* SPI Configuration */
#define CFG_SYS_SPI_CLK		ks_clk_get_rate(KS2_CLK1_6)

/* Keystone net */
#define CFG_KSNET_MAC_ID_BASE		KS2_MAC_ID_BASE_ADDR
#define CFG_KSNET_NETCP_BASE			KS2_NETCP_BASE
#define CFG_KSNET_SERDES_SGMII_BASE		KS2_SGMII_SERDES_BASE
#define CFG_KSNET_SERDES_SGMII2_BASE		KS2_SGMII_SERDES2_BASE
#define CFG_KSNET_SERDES_LANES_PER_SGMII	KS2_LANES_PER_SGMII_SERDES

/* NAND Configuration */
#define CFG_SYS_NAND_MASK_CLE		0x4000
#define CFG_SYS_NAND_MASK_ALE		0x2000
#define CFG_SYS_NAND_CS			2

#define CFG_SYS_NAND_LARGEPAGE
#define CFG_SYS_NAND_BASE_LIST		{ 0x30000000, }

/* Now for the remaining common defines */
#include <configs/ti_armv7_common.h>

/* we may include files below only after all above definitions */
#include <asm/arch/clock.h>
#ifndef CONFIG_SOC_K2G
#define CFG_SYS_HZ_CLOCK		ks_clk_get_rate(KS2_CLK1_6)
#else
#define CFG_SYS_HZ_CLOCK		get_external_clk(sys_clk)
#endif

#endif /* __CONFIG_KS2_EVM_H */
