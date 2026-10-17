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

#if defined(CONFIG_PINCTRL)
#include <dm.h>
#include <dm/device_compat.h>
#include <dm/pinctrl.h>
#include <linux/ioport.h>

/**
 * struct qe_io_plat
 *
 * @base:		Base register address
 * @num_par_io_ports	number of io ports
 */
struct qe_io_plat {
	qepio83xx_t *base;
	u32 num_io_ports;
};
#endif

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
#else
static int qe_io_of_to_plat(struct udevice *dev)
{
	struct qe_io_plat *plat = dev_get_plat(dev);
	fdt_addr_t addr;

	addr = dev_read_addr(dev);
	if (addr == FDT_ADDR_T_NONE)
		return -EINVAL;

	plat->base = (qepio83xx_t *)addr;
	if (dev_read_u32(dev, "num-ports", &plat->num_io_ports))
		return -EINVAL;

	return 0;
}

/**
 * par_io_of_config_node	config
 * @dev:	pointer to pinctrl device
 * @pio:	ofnode of pinconfig property
 */
static int par_io_of_config_node(struct udevice *dev, ofnode pio)
{
	struct qe_io_plat *plat = dev_get_plat(dev);
	qepio83xx_t *par_io = plat->base;
	const unsigned int *pio_map;
	int pio_map_len;

	pio_map = ofnode_get_property(pio, "pio-map", &pio_map_len);
	if (!pio_map)
		return -ENOENT;

	pio_map_len /= sizeof(unsigned int);
	if ((pio_map_len % 6) != 0) {
		dev_err(dev, "%s: pio-map format wrong!\n", __func__);
		return -EINVAL;
	}

	while (pio_map_len > 0) {
		/*
		 * column pio_map[5] from linux (has_irq) not
		 * supported in u-boot yet.
		 */
		qe_cfg_iopin(par_io, (u8)pio_map[0], (u8)pio_map[1],
			     (int)pio_map[2], (int)pio_map[3],
			     (int)pio_map[4]);
		pio_map += 6;
		pio_map_len -= 6;
	}
	return 0;
}

int par_io_of_config(struct udevice *dev)
{
	u32 phandle;
	ofnode pio;
	int err;

	err = ofnode_read_u32(dev_ofnode(dev), "pio-handle", &phandle);
	if (err) {
		dev_err(dev, "%s: pio-handle not available\n", __func__);
		return err;
	}

	pio = ofnode_get_by_phandle(phandle);
	if (!ofnode_valid(pio)) {
		dev_err(dev, "%s: unable to find node\n", __func__);
		return -EINVAL;
	}

	/* To Do: find pinctrl device and pass it */
	return par_io_of_config_node(NULL, pio);
}

/*
 * This is not nice!
 * pinsettings should work with "pinctrl-" properties.
 * Unfortunately on mpc83xx powerpc linux device trees
 * devices handle this with "pio-handle" properties ...
 *
 * Even worser, old board code inits all par_io
 * pins in one step, if U-Boot uses the device
 * or not. So init all par_io definitions here too
 * as linux does this also.
 */
static void config_qe_ioports(struct udevice *dev)
{
	ofnode ofn;

	for (ofn = dev_read_first_subnode(dev); ofnode_valid(ofn);
	     ofn = dev_read_next_subnode(ofn)) {
		/*
		 * ignore errors here, as may the subnode
		 * has no pio-handle
		 */
		par_io_of_config_node(dev, ofn);
	}
}

static int par_io_pinctrl_probe(struct udevice *dev)
{
	config_qe_ioports(dev);

	return 0;
}

static int par_io_pinctrl_set_state(struct udevice *dev, struct udevice *config)
{
	return 0;
}

const struct pinctrl_ops par_io_pinctrl_ops = {
	.set_state = par_io_pinctrl_set_state,
};

static const struct udevice_id par_io_pinctrl_match[] = {
	{ .compatible = "fsl,mpc8360-par_io"},
	{ /* sentinel */ }
};

U_BOOT_DRIVER(par_io_pinctrl) = {
	.name = "par-io-pinctrl",
	.id = UCLASS_PINCTRL,
	.of_match = of_match_ptr(par_io_pinctrl_match),
	.probe = par_io_pinctrl_probe,
	.of_to_plat = qe_io_of_to_plat,
	.plat_auto	= sizeof(struct qe_io_plat),
	.ops = &par_io_pinctrl_ops,
#if CONFIG_IS_ENABLED(OF_REAL)
	.flags	= DM_FLAG_PRE_RELOC,
#endif
};
#endif
