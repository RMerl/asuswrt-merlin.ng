/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2009
 * Vipin Kumar, ST Micoelectronics, vipin.kumar@st.com.
 */

#ifndef __SPR_DEFS_H__
#define __SPR_DEFS_H__

extern int spear_board_init(ulong);
extern void setfreq(unsigned int, unsigned int);
extern unsigned int setfreq_sz;

void plat_ddr_init(void);
void spear_late_init(void);

int snor_boot_selected(void);
int nand_boot_selected(void);
int pnor_boot_selected(void);
int usb_boot_selected(void);
int uart_boot_selected(void);
int tftp_boot_selected(void);
int i2c_boot_selected(void);
int spi_boot_selected(void);
int mmc_boot_selected(void);

extern u32 mpmc_conf_vals[];

struct chip_data {
	int cpufreq;
	int dramfreq;
	int dramtype;
	uchar version[32];
};

/* HW mac id in i2c memory definitions */
#define MAGIC_OFF	0x0
#define MAGIC_LEN	0x2
#define MAGIC_BYTE0	0x55
#define MAGIC_BYTE1	0xAA
#define MAC_OFF		0x2
#define MAC_LEN		0x6

#define PNOR_WIDTH_8			0
#define	PNOR_WIDTH_16			1
#define	PNOR_WIDTH_32			2
#define PNOR_WIDTH_NUM			3
#define PNOR_WIDTH_SEARCH		0xff

#endif
