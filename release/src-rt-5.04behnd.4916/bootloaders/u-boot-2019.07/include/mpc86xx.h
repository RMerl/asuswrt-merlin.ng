/*
 * Copyright 2006 Freescale Semiconductor.
 * Jeffrey Brown
 * Srikanth Srinivasan (srikanth.srinivasan@freescale.com)
 */

#ifndef	__MPC86xx_H__
#define __MPC86xx_H__

#include <asm/fsl_lbc.h>

#define EXC_OFF_SYS_RESET	0x0100	/* System reset	offset */
#define _START_OFFSET		EXC_OFF_SYS_RESET

/*
 * platform register addresses
 */

#define GUTS_SVR	(CONFIG_SYS_CCSRBAR + 0xE00A4)
#define MCM_ABCR	(CONFIG_SYS_CCSRBAR + 0x01000)
#define MCM_DBCR	(CONFIG_SYS_CCSRBAR + 0x01008)

/*
 * l2cr values.  Look in config_<BOARD>.h for the actual setup
 */
#define l2cr		 1017

#define L2CR_L2E         0x80000000 /* bit 0 - enable */
#define L2CR_L2PE        0x40000000 /* bit 1 - data parity */
#define L2CR_L2I         0x00200000 /* bit 10 - global invalidate bit */
#define L2CR_L2CTL       0x00100000 /* bit 11 - l2 ram control */
#define L2CR_L2DO        0x00010000 /* bit 15 - data-only mode */
#define L2CR_REP         0x00001000 /* bit 19 - l2 replacement alg */
#define L2CR_HWF         0x00000800 /* bit 20 - hardware flush */
#define L2CR_L2IP        0x00000001 /* global invalidate in progress */

#define HID0_XBSEN              0x00000100
#define HID0_HIGH_BAT_EN        0x00800000
#define HID0_XAEN               0x00020000

#ifndef __ASSEMBLY__

typedef struct {
	unsigned long freq_processor;
	unsigned long freq_systembus;
	unsigned long freq_localbus;
} MPC86xx_SYS_INFO;

#define l1icache_enable	icache_enable

void l2cache_enable(void);
void l1dcache_enable(void);

static __inline__ unsigned long get_hid0 (void)
{
	unsigned long hid0;
	asm volatile("mfspr %0, 1008" : "=r" (hid0) :);
	return hid0;
}

static __inline__ unsigned long get_hid1 (void)
{
	unsigned long hid1;
	asm volatile("mfspr %0, 1009" : "=r" (hid1) :);
	return hid1;
}

static __inline__ void set_hid0 (unsigned long hid0)
{
	asm volatile("mtspr 1008, %0" : : "r" (hid0));
}

static __inline__ void set_hid1 (unsigned long hid1)
{
	asm volatile("mtspr 1009, %0" : : "r" (hid1));
}


static __inline__ unsigned long get_l2cr (void)
{
   unsigned long l2cr_val;
   asm volatile("mfspr %0, 1017" : "=r" (l2cr_val) :);
   return l2cr_val;
}

void setup_ddr_bat(phys_addr_t dram_size);
extern void setup_bats(void);

#endif  /* _ASMLANGUAGE */
#endif	/* __MPC86xx_H__ */
