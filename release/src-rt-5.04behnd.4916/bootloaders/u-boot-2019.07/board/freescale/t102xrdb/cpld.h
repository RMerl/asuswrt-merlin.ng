/* SPDX-License-Identifier: GPL-2.0+ */
/**
 * Copyright 2014 Freescale Semiconductor
 *
 */

struct cpld_data {
	u8 cpld_ver;		/* 0x00 - CPLD Major Revision Register */
	u8 cpld_ver_sub;	/* 0x01 - CPLD Minor Revision Register */
	u8 hw_ver;		/* 0x02 - Hardware Revision Register */
	u8 sw_ver;		/* 0x03 - Software Revision register */
	u8 res0[12];		/* 0x04 - 0x0F - not used */
	u8 reset_ctl1;		/* 0x10 - Reset control Register1 */
	u8 reset_ctl2;		/* 0x11 - Reset control Register2 */
	u8 int_status;		/* 0x12 - Interrupt status Register */
	u8 flash_csr;		/* 0x13 - Flash control and status register */
	u8 fan_ctl_status;	/* 0x14 - Fan control and status register  */
	u8 led_ctl_status;	/* 0x15 - LED control and status register */
	u8 sfp_ctl_status;	/* 0x16 - SFP control and status register  */
	u8 misc_ctl_status;	/* 0x17 - Miscellanies ctrl & status register*/
	u8 boot_override;	/* 0x18 - Boot override register */
	u8 boot_config1;	/* 0x19 - Boot config override register*/
	u8 boot_config2;	/* 0x1A - Boot config override register*/
} cpld_data_t;


/* Pointer to the CPLD register set */

u8 cpld_read(unsigned int reg);
void cpld_write(unsigned int reg, u8 value);

#define CPLD_READ(reg) cpld_read(offsetof(struct cpld_data, reg))
#define CPLD_WRITE(reg, value)\
		cpld_write(offsetof(struct cpld_data, reg), value)

/* CPLD on IFC */
#define CPLD_LBMAP_MASK	 0x3F
#define CPLD_BANK_SEL_MASK      0x07
#define CPLD_BANK_OVERRIDE      0x40
#define CPLD_LBMAP_ALTBANK      0x44 /* BANK OR | BANK 4 */
#define CPLD_LBMAP_DFLTBANK     0x40 /* BANK OR | BANK 0 */
#define CPLD_LBMAP_RESET	0xFF
#define CPLD_LBMAP_SHIFT	0x03
#define CPLD_BOOT_SEL	   0x80

#define CPLD_PCIE_SGMII_MUX	0x80
#define CPLD_OVERRIDE_BOOT_EN	0x01
#define CPLD_OVERRIDE_MUX_EN	0x02 /* PCIE/2.5G-SGMII mux override enable */
