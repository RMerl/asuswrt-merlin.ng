/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2004 Freescale Semiconductor.
 */

#ifndef __CADMUS_H_
#define __CADMUS_H_


/*
 * CADMUS Board System Register interface.
 */

/*
 * Returns board version register.
 */
extern unsigned int get_board_version(void);

/*
 * Returns either 33000000 or 66000000 as the SYS_CLK_FREQ.
 */
extern unsigned long get_clock_freq(void);


/*
 * Returns 1 - 4, as found in the USER CSR[6:7] bits.
 */
extern unsigned int get_pci_slot(void);


/*
 * Returns PCI DUAL as found in CM_PCI[3].
 */
extern unsigned int get_pci_dual(void);


#endif	/* __CADMUS_H_ */
