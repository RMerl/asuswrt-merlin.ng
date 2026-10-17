/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2020 Cortina Access Inc.
 *
 * Configuration for Cortina-Access Presidio board
 */

#ifndef __PRESIDIO_ASIC_H
#define __PRESIDIO_ASIC_H

/* Generic Timer Definitions */
#define CFG_SYS_TIMER_RATE		25000000
#define CFG_SYS_TIMER_COUNTER	0xf4321008

/* note: arch/arm/cpu/armv8/start.S which references GICD_BASE/GICC_BASE
 * does not yet support DT. Thus define it here.
 */
#define GICD_BASE			0xf7011000
#define GICC_BASE			0xf7012000

#define CFG_SYS_TIMER_BASE		0xf4321000

/* Use external clock source */
#define PRESIDIO_APB_CLK		125000000
#define CORTINA_PER_IO_FREQ		PRESIDIO_APB_CLK

/* Cortina Serial Configuration */
#define CORTINA_UART_CLOCK		(PRESIDIO_APB_CLK)
#define CORTINA_SERIAL_PORTS		{(void *)CFG_SYS_SERIAL0, \
					 (void *)CFG_SYS_SERIAL1}

#define CFG_SYS_SERIAL0		PER_UART0_CFG
#define CFG_SYS_SERIAL1		PER_UART1_CFG

/* SDRAM Bank #1 */
#define DDR_BASE			0x00000000
#define PHYS_SDRAM_1			DDR_BASE
#define PHYS_SDRAM_1_SIZE		0x80000000 /* 2GB */
#define CFG_SYS_SDRAM_BASE		PHYS_SDRAM_1

/* Console I/O Buffer Size */

#define KSEG1_ATU_XLAT(x) (x)

/* HW REG ADDR */
#define NI_READ_POLL_COUNT                      1000
#define CA_NI_MDIO_REG_BASE                     0xF4338
#define NI_HV_GLB_MAC_ADDR_CFG0_OFFSET          0x010
#define NI_HV_GLB_MAC_ADDR_CFG1_OFFSET          0x014
#define NI_HV_PT_BASE                           0x400
#define NI_HV_XRAM_BASE                         0x820
#define GLOBAL_BLOCK_RESET_OFFSET               0x04
#define GLOBAL_GLOBAL_CONFIG_OFFSET             0x20
#define GLOBAL_IO_DRIVE_CONTROL_OFFSET          0x4c

/* max command args */
#define CFG_EXTRA_ENV_SETTINGS	"silent=y\0"

/* nand driver parameters */
#ifdef CONFIG_TARGET_PRESIDIO_ASIC
	#define CFG_SYS_NAND_BASE            CFG_SYS_FLASH_BASE
	#define CFG_SYS_NAND_BASE_LIST       { CFG_SYS_NAND_BASE }
#endif

#endif /* __PRESIDIO_ASIC_H */
