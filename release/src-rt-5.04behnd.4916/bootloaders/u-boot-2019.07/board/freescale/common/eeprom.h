/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2004 Freescale Semiconductor.
 */

#ifndef __EEPROM_H_
#define __EEPROM_H_


/*
 * EEPROM Board System Register interface.
 */


/*
 * CPU Board Revision
 */
#define MPC85XX_CPU_BOARD_REV(maj, min)	((((maj)&0xff) << 8) | ((min) & 0xff))
#define MPC85XX_CPU_BOARD_MAJOR(rev)	(((rev) >> 8) & 0xff)
#define MPC85XX_CPU_BOARD_MINOR(rev)	((rev) & 0xff)

#define MPC85XX_CPU_BOARD_REV_UNKNOWN	MPC85XX_CPU_BOARD_REV(0,0)
#define MPC85XX_CPU_BOARD_REV_1_0	MPC85XX_CPU_BOARD_REV(1,0)
#define MPC85XX_CPU_BOARD_REV_1_1	MPC85XX_CPU_BOARD_REV(1,1)

/*
 * Returns CPU board revision register as a 16-bit value with
 * the Major in the high byte, and Minor in the low byte.
 */
extern unsigned int get_cpu_board_revision(void);


#endif	/* __CADMUS_H_ */
