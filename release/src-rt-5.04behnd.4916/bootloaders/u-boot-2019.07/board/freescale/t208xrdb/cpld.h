/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2014 Freescale Semiconductor
 */

/*
 * CPLD register set of T2080RDB board-specific.
 */
struct cpld_data {
	u8 chip_id1;		/* 0x00 - Chip ID1 register */
	u8 chip_id2;		/* 0x01 - Chip ID2 register */
	u8 hw_ver;		/* 0x02 - Hardware Revision Register */
	u8 sw_ver;		/* 0x03 - Software Revision register */
	u8 res0[12];		/* 0x04 - 0x0F - not used */
	u8 reset_ctl;		/* 0x10 - Reset control Register */
	u8 flash_csr;		/* 0x11 - Flash control and status register */
	u8 thermal_csr;		/* 0x12 - Thermal control and status register */
	u8 led_csr;		/* 0x13 - LED control and status register */
	u8 sfp_csr;		/* 0x14 - SFP+ control and status register */
	u8 misc_csr;		/* 0x15 - Misc control and status register */
	u8 boot_or;		/* 0x16 - Boot config override register */
	u8 boot_cfg1;		/* 0x17 - Boot configuration register 1 */
	u8 boot_cfg2;		/* 0x18 - Boot configuration register 2 */
} cpld_data_t;

u8 cpld_read(unsigned int reg);
void cpld_write(unsigned int reg, u8 value);

#define CPLD_READ(reg) cpld_read(offsetof(struct cpld_data, reg))
#define CPLD_WRITE(reg, value)  \
	cpld_write(offsetof(struct cpld_data, reg), value)

/* CPLD on IFC */
#define CPLD_LBMAP_MASK		0x3F
#define CPLD_BANK_SEL_MASK	0x07
#define CPLD_BANK_OVERRIDE	0x40
#define CPLD_LBMAP_ALTBANK	0x44 /* BANK OR | BANK 4 */
#define CPLD_LBMAP_DFLTBANK	0x40 /* BANK OR | BANK 0 */
#define CPLD_LBMAP_RESET	0xFF
#define CPLD_LBMAP_SHIFT	0x03
#define CPLD_BOOT_SEL		0x80

/* RSTCON Register */
#define CPLD_RSTCON_EDC_RST	0x04
