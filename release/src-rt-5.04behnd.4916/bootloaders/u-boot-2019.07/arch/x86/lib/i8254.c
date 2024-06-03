// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2002
 * Daniel Engström, Omicron Ceti AB, <daniel@omicron.se>
 */

#include <common.h>
#include <asm/io.h>
#include <asm/i8254.h>

#define TIMER1_VALUE		18	/* 15.6us */
#define BEEP_FREQUENCY_HZ	440
#define SYSCTL_PORTB		0x61
#define PORTB_BEEP_ENABLE	0x3

static void i8254_set_beep_freq(uint frequency_hz)
{
	uint countdown;

	countdown = PIT_TICK_RATE / frequency_hz;

	outb(countdown & 0xff, PIT_BASE + PIT_T2);
	outb((countdown >> 8) & 0xff, PIT_BASE + PIT_T2);
}

int i8254_init(void)
{
	/*
	 * Initialize counter 1, used to refresh request signal.
	 * This is required for legacy purpose as some codes like
	 * vgabios utilizes counter 1 to provide delay functionality.
	 */
	outb(PIT_CMD_CTR1 | PIT_CMD_LOW | PIT_CMD_MODE2,
	     PIT_BASE + PIT_COMMAND);
	outb(TIMER1_VALUE, PIT_BASE + PIT_T1);

	/*
	 * Initialize counter 2, used to drive the speaker.
	 * To start a beep, set both bit0 and bit1 of port 0x61.
	 * To stop it, clear both bit0 and bit1 of port 0x61.
	 */
	outb(PIT_CMD_CTR2 | PIT_CMD_BOTH | PIT_CMD_MODE3,
	     PIT_BASE + PIT_COMMAND);
	i8254_set_beep_freq(BEEP_FREQUENCY_HZ);

	return 0;
}

int i8254_enable_beep(uint frequency_hz)
{
	if (!frequency_hz)
		return -EINVAL;

	/* make sure i8254 is setup correctly before generating beeps */
	outb(PIT_CMD_CTR2 | PIT_CMD_BOTH | PIT_CMD_MODE3,
	     PIT_BASE + PIT_COMMAND);

	i8254_set_beep_freq(frequency_hz);
	setio_8(SYSCTL_PORTB, PORTB_BEEP_ENABLE);

	return 0;
}

void i8254_disable_beep(void)
{
	clrio_8(SYSCTL_PORTB, PORTB_BEEP_ENABLE);
}
