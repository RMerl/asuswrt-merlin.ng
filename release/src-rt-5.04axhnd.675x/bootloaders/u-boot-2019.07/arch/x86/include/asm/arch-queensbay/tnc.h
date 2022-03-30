/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2014, Bin Meng <bmeng.cn@gmail.com>
 */

#ifndef _X86_ARCH_TNC_H_
#define _X86_ARCH_TNC_H_

/* IGD Function Disable Register */
#define IGD_FD		0xc4
#define FUNC_DISABLE	0x00000001

/* Memory BAR Enable */
#define MEM_BAR_EN	0x00000001

/* LPC PCI Configuration Registers */
#define LPC_RCBA	0xf0

/* Root Complex Register Block */
struct tnc_rcba {
	u32	rctl;
	u32	esd;
	u32	rsvd1[2];
	u32	hdd;
	u32	rsvd2;
	u32	hdba;
	u32	rsvd3[3129];
	u32	d31ip;
	u32	rsvd4[3];
	u32	d27ip;
	u32	rsvd5;
	u32	d02ip;
	u32	rsvd6;
	u32	d26ip;
	u32	d25ip;
	u32	d24ip;
	u32	d23ip;
	u32	d03ip;
	u32	rsvd7[3];
	u16	d31ir;
	u16	rsvd8[3];
	u16	d27ir;
	u16	d26ir;
	u16	d25ir;
	u16	d24ir;
	u16	d23ir;
	u16	rsvd9[7];
	u16	d02ir;
	u16	d03ir;
};

#endif /* _X86_ARCH_TNC_H_ */
