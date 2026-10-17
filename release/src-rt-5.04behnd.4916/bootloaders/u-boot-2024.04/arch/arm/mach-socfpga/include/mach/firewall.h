/* SPDX-License-Identifier: GPL-2.0
 *
 * Copyright (C) 2017-2019 Intel Corporation <www.intel.com>
 *
 */

#ifndef	_FIREWALL_H_
#define	_FIREWALL_H_

#include <linux/bitops.h>

struct socfpga_firwall_l4_per {
	u32	nand;		/* 0x00 */
	u32	nand_data;
	u32	_pad_0x8;
	u32	usb0;
	u32	usb1;		/* 0x10 */
	u32	_pad_0x14;
	u32	_pad_0x18;
	u32	spim0;
	u32	spim1;		/* 0x20 */
	u32	spis0;
	u32	spis1;
	u32	emac0;
	u32	emac1;		/* 0x30 */
	u32	emac2;
	u32	_pad_0x38;
	u32	_pad_0x3c;
	u32	sdmmc;		/* 0x40 */
	u32	gpio0;
	u32	gpio1;
	u32	_pad_0x4c;
	u32	i2c0;		/* 0x50 */
	u32	i2c1;
	u32	i2c2;
	u32	i2c3;
	u32	i2c4;		/* 0x60 */
	u32	timer0;
	u32	timer1;
	u32	uart0;
	u32	uart1;		/* 0x70 */
};

struct socfpga_firwall_l4_sys {
	u32	_pad_0x00;		/* 0x00 */
	u32	_pad_0x04;
	u32	dma_ecc;
	u32	emac0rx_ecc;
	u32	emac0tx_ecc;		/* 0x10 */
	u32	emac1rx_ecc;
	u32	emac1tx_ecc;
	u32	emac2rx_ecc;
	u32	emac2tx_ecc;		/* 0x20 */
	u32	_pad_0x24;
	u32	_pad_0x28;
	u32	nand_ecc;
	u32	nand_read_ecc;		/* 0x30 */
	u32	nand_write_ecc;
	u32	ocram_ecc;
	u32	_pad_0x3c;
	u32	sdmmc_ecc;		/* 0x40 */
	u32	usb0_ecc;
	u32	usb1_ecc;
	u32	clock_manager;
	u32	_pad_0x50;		/* 0x50 */
	u32	io_manager;
	u32	reset_manager;
	u32	system_manager;
	u32	osc0_timer;		/* 0x60 */
	u32	osc1_timer;
	u32	watchdog0;
	u32	watchdog1;
	u32	watchdog2;		/* 0x70 */
	u32	watchdog3;
};

#define FIREWALL_L4_DISABLE_ALL		(BIT(0) | BIT(24) | BIT(16))
#define FIREWALL_BRIDGE_DISABLE_ALL	(~0)

/* Cache coherency unit (CCU) registers */
#define CCU_CPU0_MPRT_ADBASE_DDRREG		0x4400
#define CCU_CPU0_MPRT_ADBASE_MEMSPACE0		0x45c0
#define CCU_CPU0_MPRT_ADBASE_MEMSPACE1A		0x45e0
#define CCU_CPU0_MPRT_ADBASE_MEMSPACE1B		0x4600
#define CCU_CPU0_MPRT_ADBASE_MEMSPACE1C		0x4620
#define CCU_CPU0_MPRT_ADBASE_MEMSPACE1D		0x4640
#define CCU_CPU0_MPRT_ADBASE_MEMSPACE1E		0x4660

#define CCU_CPU0_MPRT_ADMASK_MEM_RAM0		0x4688

#define CCU_IOM_MPRT_ADBASE_MEMSPACE0		0x18560
#define CCU_IOM_MPRT_ADBASE_MEMSPACE1A		0x18580
#define CCU_IOM_MPRT_ADBASE_MEMSPACE1B		0x185a0
#define CCU_IOM_MPRT_ADBASE_MEMSPACE1C		0x185c0
#define CCU_IOM_MPRT_ADBASE_MEMSPACE1D		0x185e0
#define CCU_IOM_MPRT_ADBASE_MEMSPACE1E		0x18600

#define CCU_IOM_MPRT_ADMASK_MEM_RAM0		0x18628

#define CCU_TCU_MPRT_ADBASE_MEMSPACE0		0x2c520
#define CCU_TCU_MPRT_ADBASE_MEMSPACE1A		0x2c540
#define CCU_TCU_MPRT_ADBASE_MEMSPACE1B		0x2c560
#define CCU_TCU_MPRT_ADBASE_MEMSPACE1C		0x2c580
#define CCU_TCU_MPRT_ADBASE_MEMSPACE1D		0x2c5a0
#define CCU_TCU_MPRT_ADBASE_MEMSPACE1E		0x2c5c0

#define CCU_ADMASK_P_MASK			BIT(0)
#define CCU_ADMASK_NS_MASK			BIT(1)

#define CCU_ADBASE_DI_MASK			BIT(4)

#define CCU_REG_ADDR(reg)			\
	(SOCFPGA_CCU_ADDRESS + (reg))

/* Firewall MPU DDR SCR registers */
#define FW_MPU_DDR_SCR_EN				0x00
#define FW_MPU_DDR_SCR_EN_SET				0x04
#define FW_MPU_DDR_SCR_MPUREGION0ADDR_BASE		0x10
#define FW_MPU_DDR_SCR_MPUREGION0ADDR_BASEEXT		0x14
#define FW_MPU_DDR_SCR_MPUREGION0ADDR_LIMIT		0x18
#define FW_MPU_DDR_SCR_MPUREGION0ADDR_LIMITEXT		0x1c

#define FW_MPU_DDR_SCR_NONMPUREGION0ADDR_BASE		0x90
#define FW_MPU_DDR_SCR_NONMPUREGION0ADDR_BASEEXT	0x94
#define FW_MPU_DDR_SCR_NONMPUREGION0ADDR_LIMIT		0x98
#define FW_MPU_DDR_SCR_NONMPUREGION0ADDR_LIMITEXT	0x9c
#define FW_MPU_DDR_SCR_NONMPUREGION0ADDR_LIMITEXT_FIELD	0xff

#define MPUREGION0_ENABLE				BIT(0)
#define NONMPUREGION0_ENABLE				BIT(8)

#define FW_MPU_DDR_SCR_WRITEL(data, reg)		\
	writel(data, SOCFPGA_FW_MPU_DDR_SCR_ADDRESS + (reg))

void firewall_setup(void);

#endif /* _FIREWALL_H_ */
