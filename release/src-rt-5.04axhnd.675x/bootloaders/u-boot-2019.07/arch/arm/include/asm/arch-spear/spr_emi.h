/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2009
 * Ryan CHEN, ST Micoelectronics, ryan.chen@st.com
 */

#ifndef __SPEAR_EMI_H__
#define __SPEAR_EMI_H__

#ifdef CONFIG_SPEAR_EMI

struct emi_bank_regs {
	u32 tap;
	u32 tsdp;
	u32 tdpw;
	u32 tdpr;
	u32 tdcs;
	u32 control;
};

struct emi_regs {
	struct emi_bank_regs bank_regs[CONFIG_SYS_MAX_FLASH_BANKS];
	u32 tout;
	u32 ack;
	u32 irq;
};

#define EMI_ACKMSK		0x40

/* control register definitions */
#define EMI_CNTL_ENBBYTEW	(1 << 2)
#define EMI_CNTL_ENBBYTER	(1 << 3)
#define EMI_CNTL_ENBBYTERW	(EMI_CNTL_ENBBYTER | EMI_CNTL_ENBBYTEW)

#endif

#endif
