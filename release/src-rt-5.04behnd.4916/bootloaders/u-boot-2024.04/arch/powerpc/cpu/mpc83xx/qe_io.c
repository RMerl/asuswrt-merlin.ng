// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2006 Freescale Semiconductor, Inc.
 *
 * Dave Liu <daveliu@freescale.com>
 * based on source code of Shlomi Gridish
 */

#include <common.h>
#include <linux/errno.h>
#include <asm/io.h>
#include <asm/immap_83xx.h>

#define	NUM_OF_PINS	32

/** qe_cfg_iopin configure one io pin setting
 *
 * @par_io:	pointer to parallel I/O base
 * @port:	io pin port
 * @pin:	io pin number which get configured
 * @dir:	direction of io pin 2 bits valid
 *		00 = pin disabled
 *		01 = output
 *		10 = input
 *		11 = pin is I/O
 * @open_drain:	is pin open drain
 * @assign:	pin assignment registers select the function of the pin
 */
static void qe_cfg_iopin(qepio83xx_t *par_io, u8 port, u8 pin, int dir,
			 int open_drain, int assign)
{
	u32	dbit_mask;
	u32	dbit_dir;
	u32	dbit_asgn;
	u32	bit_mask;
	u32	tmp_val;
	int	offset;

	offset = (NUM_OF_PINS - (pin % (NUM_OF_PINS / 2) + 1) * 2);

	/* Calculate pin location and 2bit mask and dir */
	dbit_mask = (u32)(0x3 << offset);
	dbit_dir = (u32)(dir << offset);

	/* Setup the direction */
	tmp_val = (pin > (NUM_OF_PINS / 2) - 1) ?
		in_be32(&par_io->ioport[port].dir2) :
		in_be32(&par_io->ioport[port].dir1);

	if (pin > (NUM_OF_PINS / 2) - 1) {
		out_be32(&par_io->ioport[port].dir2, ~dbit_mask & tmp_val);
		out_be32(&par_io->ioport[port].dir2, dbit_dir | tmp_val);
	} else {
		out_be32(&par_io->ioport[port].dir1, ~dbit_mask & tmp_val);
		out_be32(&par_io->ioport[port].dir1, dbit_dir | tmp_val);
	}

	/* Calculate pin location for 1bit mask */
	bit_mask = (u32)(1 << (NUM_OF_PINS - (pin + 1)));

	/* Setup the open drain */
	tmp_val = in_be32(&par_io->ioport[port].podr);
	if (open_drain)
		out_be32(&par_io->ioport[port].podr, bit_mask | tmp_val);
	else
		out_be32(&par_io->ioport[port].podr, ~bit_mask & tmp_val);

	/* Setup the assignment */
	tmp_val = (pin > (NUM_OF_PINS / 2) - 1) ?
		in_be32(&par_io->ioport[port].ppar2) :
		in_be32(&par_io->ioport[port].ppar1);
	dbit_asgn = (u32)(assign << offset);

	/* Clear and set 2 bits mask */
	if (pin > (NUM_OF_PINS / 2) - 1) {
		out_be32(&par_io->ioport[port].ppar2, ~dbit_mask & tmp_val);
		out_be32(&par_io->ioport[port].ppar2, dbit_asgn | tmp_val);
	} else {
		out_be32(&par_io->ioport[port].ppar1, ~dbit_mask & tmp_val);
		out_be32(&par_io->ioport[port].ppar1, dbit_asgn | tmp_val);
	}
}

#if !defined(CONFIG_PINCTRL)
/** qe_config_iopin configure one io pin setting
 *
 * @port:	io pin port
 * @pin:	io pin number which get configured
 * @dir:	direction of io pin 2 bits valid
 *		00 = pin disabled
 *		01 = output
 *		10 = input
 *		11 = pin is I/O
 * @open_drain:	is pin open drain
 * @assign:	pin assignment registers select the function of the pin
 */
void qe_config_iopin(u8 port, u8 pin, int dir, int open_drain, int assign)
{
	immap_t        *im = (immap_t *)CONFIG_SYS_IMMR;
	qepio83xx_t    *par_io = (qepio83xx_t *)&im->qepio;

	qe_cfg_iopin(par_io, port, pin, dir, open_drain, assign);
}
#endif
