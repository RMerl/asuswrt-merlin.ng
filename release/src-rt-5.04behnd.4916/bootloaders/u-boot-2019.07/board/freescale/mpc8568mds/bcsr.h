/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2007 Freescale Semiconductor.
 */

#ifndef __BCSR_H_
#define __BCSR_H_

#include <common.h>

/* BCSR Bit definitions
	* BCSR 0 *
	0:3	ccb sys pll
	4:6	cfg core pll
	7	cfg boot seq

	* BCSR 1 *
	0:2	cfg rom lock
	3:5	cfg host agent
	6	PCI IO
	7	cfg RIO size

	* BCSR 2 *
	0:4	QE PLL
	5	QE clock
	6	cfg PCI arbiter

	* BCSR 3 *
	0	TSEC1 reduce
	1	TSEC2 reduce
	2:3	TSEC1 protocol
	4:5	TSEC2 protocol
	6	PHY1 slave
	7	PHY2 slave

	* BCSR 4 *
	4	clock enable
	5	boot EPROM
	6	GETH transactive reset
	7	BRD write potect

	* BCSR 5 *
	1:3	Leds 1-3
	4	UPC1 enable
	5	UPC2 enable
	6	UPC2 pos
	7	RS232 enable

	* BCSR 6 *
	0	CFG ver 0
	1	CFG ver 1
	6	Register config led
	7	Power on reset

	* BCSR 7 *
	2	board host mode indication
	5	enable TSEC1 PHY
	6	enable TSEC2 PHY

	* BCSR 8 *
	0	UCC GETH1 enable
	1	UCC GMII enable
	3	UCC TBI enable
	5	UCC MII enable
	7	Real time clock reset

	* BCSR 9 *
	0	UCC2 GETH enable
	1	UCC2 GMII enable
	3	UCC2 TBI enable
	5	UCC2 MII enable
	6	Ready only - indicate flash ready after burning
	7	Flash write protect
*/

#define BCSR_UCC1_GETH_EN	(0x1 << 7)
#define BCSR_UCC2_GETH_EN	(0x1 << 7)
#define BCSR_UCC1_MODE_MSK	(0x3 << 4)
#define BCSR_UCC2_MODE_MSK	(0x3 << 0)

/*BCSR Utils functions*/

void enable_8568mds_duart(void);
void enable_8568mds_flash_write(void);
void disable_8568mds_flash_write(void);
void enable_8568mds_qe_mdio(void);

#if defined(CONFIG_UEC_ETH1) || defined(CONFIG_UEC_ETH2)
void reset_8568mds_uccs(void);
#endif

#endif	/* __BCSR_H_ */
