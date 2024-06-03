/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2002
 * Daniel Engström, Omicron Ceti AB, daniel@omicron.se.
 */

/* i8254.h Intel 8254 PIT registers */

#ifndef _ASMI386_I8254_H_
#define _ASMI386_I8954_H_

#define PIT_T0		0x00	/* PIT channel 0 count/status */
#define PIT_T1		0x01	/* PIT channel 1 count/status */
#define PIT_T2		0x02	/* PIT channel 2 count/status */
#define PIT_COMMAND	0x03	/* PIT mode control, latch and read back */

/* PIT Command Register Bit Definitions */

#define PIT_CMD_CTR0	0x00	/* Select PIT counter 0 */
#define PIT_CMD_CTR1	0x40	/* Select PIT counter 1 */
#define PIT_CMD_CTR2	0x80	/* Select PIT counter 2 */

#define PIT_CMD_LATCH	0x00	/* Counter Latch Command */
#define PIT_CMD_LOW	0x10	/* Access counter bits 7-0 */
#define PIT_CMD_HIGH	0x20	/* Access counter bits 15-8 */
#define PIT_CMD_BOTH	0x30	/* Access counter bits 15-0 in two accesses */

#define PIT_CMD_MODE0	0x00	/* Select mode 0 */
#define PIT_CMD_MODE1	0x02	/* Select mode 1 */
#define PIT_CMD_MODE2	0x04	/* Select mode 2 */
#define PIT_CMD_MODE3	0x06	/* Select mode 3 */
#define PIT_CMD_MODE4	0x08	/* Select mode 4 */
#define PIT_CMD_MODE5	0x0a	/* Select mode 5 */

/* The clock frequency of the i8253/i8254 PIT */
#define PIT_TICK_RATE	1193182

/**
 * i8254_enable_beep() - Start a beep using the PCAT timer
 *
 * This starts beeping using the legacy i8254 timer. The beep may be silenced
 * after a delay with i8254_disable_beep().
 *
 * @frequency_hz: Frequency of beep in Hz
 * @return 0 if OK, -EINVAL if frequency_hz is 0
 */
int i8254_enable_beep(uint frequency_hz);

/**
 * i8254_disable_beep() - Disable the bepper
 *
 * This stops any existing beep
 */
void i8254_disable_beep(void);

#endif /* _ASMI386_I8954_H_ */
