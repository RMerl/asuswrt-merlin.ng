/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2014  Angelo Dureghello <angelo@sysam.it>
 *
 */

#ifndef __IMMAP_5307__
#define __IMMAP_5307__

#define MMAP_SIM	(CONFIG_SYS_MBAR + 0x00000000)
#define MMAP_INTC	(CONFIG_SYS_MBAR + 0x00000040)
#define MMAP_CSM	(CONFIG_SYS_MBAR + 0x00000080)
#define MMAP_DRAMC	(CONFIG_SYS_MBAR + 0x00000100)
#define MMAP_DTMR0	(CONFIG_SYS_MBAR + 0x00000140)
#define MMAP_DTMR1	(CONFIG_SYS_MBAR + 0x00000180)
#define MMAP_UART0	(CONFIG_SYS_MBAR + 0x000001C0)
#define MMAP_UART1	(CONFIG_SYS_MBAR + 0x00000200)
#define MMAP_GPIO	(CONFIG_SYS_MBAR + 0x00000244)

typedef struct sim {
	u8  rsr;
	u8  sypcr;
	u8  swivr;
	u8  swsr;
	u16 par;
	u8  irqpar;
	u8  res1;
	u8  pllcr;
	u8  res2;
	u16 res3;
	u8  mpark;
	u8  res4;
	u16 res5;
	u32 res6;
} sim_t;

typedef struct intctrl {
	u32 ipr;
	u32 imr;
	u16 res7;
	u8  res8;
	u8  avr;
	u8  icr0;
	u8  icr1;
	u8  icr2;
	u8  icr3;
	u8  icr4;
	u8  icr5;
	u8  icr6;
	u8  icr7;
	u8  icr8;
	u8  icr9;
	u16 res9;
} intctrl_t;

typedef struct csm {
	u16 csar0;      /* Chip-select Address */
	u16 res1;
	u32 csmr0;      /* Chip-select Mask */
	u16 res2;
	u16 cscr0;      /* Chip-select Control */
	u16 csar1;
	u16 res3;
	u32 csmr1;
	u16 res4;
	u16 cscr1;
	u16 csar2;
	u16 res5;
	u32 csmr2;
	u16 res6;
	u16 cscr2;
	u16 csar3;
	u16 res7;
	u32 csmr3;
	u16 res8;
	u16 cscr3;
	u16 csar4;
	u16 res9;
	u32 csmr4;
	u16 res10;
	u16 cscr4;
	u16 csar5;
	u16 res11;
	u32 csmr5;
	u16 res12;
	u16 cscr5;
	u16 csar6;
	u16 res13;
	u32 csmr6;
	u16 res14;
	u16 cscr6;
	u16 csar7;
	u16 res15;
	u32 csmr7;
	u16 res16;
	u16 cscr7;
} csm_t;

typedef struct sdramctrl {
	u16 dcr;
	u16 res1;
	u32 res2;
	u32 dacr0;
	u32 dmr0;
	u32 dacr1;
	u32 dmr1;
} sdramctrl_t;

typedef struct gpio {
	u16 paddr;
	u16 res1;
	u16 padat;
	u16 res2;
} gpio_t;

#endif				/* __IMMAP_5307__ */

