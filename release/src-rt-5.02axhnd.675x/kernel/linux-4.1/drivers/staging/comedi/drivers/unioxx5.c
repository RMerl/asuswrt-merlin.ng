/***************************************************************************
 *                                                                         *
 *  comedi/drivers/unioxx5.c                                               *
 *  Driver for Fastwel UNIOxx-5 (analog and digital i/o) boards.           *
 *                                                                         *
 *  Copyright (C) 2006 Kruchinin Daniil (asgard) [asgard@etersoft.ru]      *
 *                                                                         *
 *  COMEDI - Linux Control and Measurement Device Interface                *
 *  Copyright (C) 1998,2000 David A. Schleef <ds@schleef.org>              *
 *                                                                         *
 *  This program is free software; you can redistribute it and/or modify   *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation; either version 2 of the License, or      *
 *  (at your option) any later version.                                    *
 *                                                                         *
 *  This program is distributed in the hope that it will be useful,        *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 ***************************************************************************/
/*

Driver: unioxx5
Description: Driver for Fastwel UNIOxx-5 (analog and digital i/o) boards.
Author: Kruchinin Daniil (asgard) <asgard@etersoft.ru>
Status: unknown
Updated: 2006-10-09
Devices: [Fastwel] UNIOxx-5 (unioxx5),

 This card supports digital and analog I/O. It written for g01
 subdevices only.
 channels range: 0 .. 23 dio channels
 and 0 .. 11 analog modules range
 During attaching unioxx5 module displays modules identifiers
 (see dmesg after comedi_config) in format:
 | [module_number] module_id |

*/

#include <linux/module.h>
#include <linux/delay.h>
#include "../comedidev.h"

#define UNIOXX5_SIZE 0x10
#define UNIOXX5_SUBDEV_BASE 0xA000	/* base addr of first subdev */
#define UNIOXX5_SUBDEV_ODDS 0x400

/* modules types */
#define MODULE_DIGITAL 0
#define MODULE_OUTPUT_MASK 0x80	/* analog input/output */

/* constants for digital i/o */
#define UNIOXX5_NUM_OF_CHANS 24

/* constants for analog i/o */
#define TxBE  0x10		/* transmit buffer enable */
#define RxCA  0x20		/* 1 receive character available */
#define Rx2CA 0x40		/* 2 receive character available */
#define Rx4CA 0x80		/* 4 receive character available */

/* bytes mask errors */
#define Rx2CA_ERR_MASK 0x04	/* 2 bytes receiving error */
#define Rx4CA_ERR_MASK 0x08	/* 4 bytes receiving error */

/* channel modes */
#define ALL_2_INPUT  0		/* config all digital channels to input */
#define ALL_2_OUTPUT 1		/* config all digital channels to output */

/* 'private' structure for each subdevice */
struct unioxx5_subd_priv {
	int usp_iobase;
	/* 12 modules. each can be 70L or 73L */
	unsigned char usp_module_type[12];
	/* for saving previous written value for analog modules */
	unsigned char usp_extra_data[12][4];
	unsigned char usp_prev_wr_val[3];	/* previous written value */
	unsigned char usp_prev_cn_val[3];	/* previous channel value */
};

static int __unioxx5_define_chan_offset(int chan_num)
{
	if (chan_num < 0 || chan_num > 23)
		return -1;

	return (chan_num >> 3) + 1;
}

#if 0				/* not used? */
static void __unioxx5_digital_config(struct comedi_subdevice *s, int mode)
{
	struct unioxx5_subd_priv *usp = s->private;
	struct device *csdev = s->device->class_dev;
	int i, mask;

	mask = (mode == ALL_2_OUTPUT) ? 0xFF : 0x00;
	dev_dbg(csdev, "mode = %d\n", mask);

	outb(1, usp->usp_iobase + 0);

	for (i = 0; i < 3; i++)
		outb(mask, usp->usp_iobase + i);

	outb(0, usp->usp_iobase + 0);
}
#endif

/* configure channels for analog i/o (even to output, odd to input) */
static void __unioxx5_analog_config(struct unioxx5_subd_priv *usp, int channel)
{
	int chan_a, chan_b, conf, channel_offset;

	channel_offset = __unioxx5_define_chan_offset(channel);
	conf = usp->usp_prev_cn_val[channel_offset - 1];
	chan_a = chan_b = 1;

	/* setting channel A and channel B mask */
	if (channel % 2 == 0) {
		chan_a <<= channel & 0x07;
		chan_b <<= (channel + 1) & 0x07;
	} else {
		chan_a <<= (channel - 1) & 0x07;
		chan_b <<= channel & 0x07;
	}

	conf |= chan_a;		/* even channel ot output */
	conf &= ~chan_b;	/* odd channel to input */

	outb(1, usp->usp_iobase + 0);
	outb(conf, usp->usp_iobase + channel_offset);
	outb(0, usp->usp_iobase + 0);

	usp->usp_prev_cn_val[channel_offset - 1] = conf;
}

static int __unioxx5_digital_read(struct comedi_subdevice *s,
				  unsigned int *data, int channel, int minor)
{
	struct unioxx5_subd_priv *usp = s->private;
	struct device *csdev = s->device->class_dev;
	int channel_offset, mask = 1 << (channel & 0x07);

	channel_offset = __unioxx5_define_chan_offset(channel);
	if (channel_offset < 0) {
		dev_err(csdev,
			"undefined channel %d. channel range is 0 .. 23\n",
			channel);
		return 0;
	}

	*data = inb(usp->usp_iobase + channel_offset);
	*data &= mask;

	/* correct the read value to 0 or 1 */
	if (channel_offset > 1)
		channel -= 2 << channel_offset;
	*data >>= channel;
	return 1;
}

static int __unioxx5_analog_read(struct comedi_subdevice *s,
				 unsigned int *data, int channel, int minor)
{
	struct unioxx5_subd_priv *usp = s->private;
	struct device *csdev = s->device->class_dev;
	int module_no, read_ch;
	char control;

	module_no = channel / 2;
	read_ch = channel % 2;	/* depend on type of channel (A or B) */

	/* defining if given module can work on input */
	if (usp->usp_module_type[module_no] & MODULE_OUTPUT_MASK) {
		dev_err(csdev,
			"module in position %d with id 0x%02x is for output only",
			module_no, usp->usp_module_type[module_no]);
		return 0;
	}

	__unioxx5_analog_config(usp, channel);
	/* sends module number to card(1 .. 12) */
	outb(module_no + 1, usp->usp_iobase + 5);
	outb('V', usp->usp_iobase + 6);	/* sends to module (V)erify command */
	control = inb(usp->usp_iobase);	/* get control register byte */

	/* waits while reading four bytes will be allowed */
	while (!((control = inb(usp->usp_iobase + 0)) & Rx4CA))
		;

	/* if four bytes readding error occurs - return 0(false) */
	if ((control & Rx4CA_ERR_MASK)) {
		dev_err(csdev, "4 bytes error\n");
		return 0;
	}

	if (read_ch)
		*data = inw(usp->usp_iobase + 6);	/* channel B */
	else
		*data = inw(usp->usp_iobase + 4);	/* channel A */

	return 1;
}

static int __unioxx5_digital_write(struct comedi_subdevice *s,
				   unsigned int *data, int channel, int minor)
{
	struct unioxx5_subd_priv *usp = s->private;
	struct device *csdev = s->device->class_dev;
	int channel_offset, val;
	int mask = 1 << (channel & 0x07);

	channel_offset = __unioxx5_define_chan_offset(channel);
	if (channel_offset < 0) {
		dev_err(csdev,
			"undefined channel %d. channel range is 0 .. 23\n",
			channel);
		return 0;
	}

	/* getting previous written value */
	val = usp->usp_prev_wr_val[channel_offset - 1];

	if (*data)
		val |= mask;
	else
		val &= ~mask;

	outb(val, usp->usp_iobase + channel_offset);
	/* saving new written value */
	usp->usp_prev_wr_val[channel_offset - 1] = val;

	return 1;
}

static int __unioxx5_analog_write(struct comedi_subdevice *s,
				  unsigned int *data, int channel, int minor)
{
	struct unioxx5_subd_priv *usp = s->private;
	struct device *csdev = s->device->class_dev;
	int module, i;

	module = channel / 2;	/* definig module number(0 .. 11) */
	i = (channel % 2) << 1;	/* depends on type of channel (A or B) */

	/* defining if given module can work on output */
	if (!(usp->usp_module_type[module] & MODULE_OUTPUT_MASK)) {
		dev_err(csdev,
			"module in position %d with id 0x%0x is for input only!\n",
			module, usp->usp_module_type[module]);
		return 0;
	}

	__unioxx5_analog_config(usp, channel);
	/* saving minor byte */
	usp->usp_extra_data[module][i++] = (unsigned char)(*data & 0x00FF);
	/* saving major byte */
	usp->usp_extra_data[module][i] = (unsigned char)((*data & 0xFF00) >> 8);

	/* while(!((inb(usp->usp_iobase + 0)) & TxBE)); */
	/* sending module number to card(1 .. 12) */
	outb(module + 1, usp->usp_iobase + 5);
	outb('W', usp->usp_iobase + 6);	/* sends (W)rite command to module */

	/* sending for bytes to module(one byte per cycle iteration) */
	for (i = 0; i < 4; i++) {
		while (!((inb(usp->usp_iobase + 0)) & TxBE))
			;	/* waits while writing will be allowed */
		outb(usp->usp_extra_data[module][i], usp->usp_iobase + 6);
	}

	return 1;
}

static int unioxx5_subdev_read(struct comedi_device *dev,
			       struct comedi_subdevice *subdev,
			       struct comedi_insn *insn, unsigned int *data)
{
	struct unioxx5_subd_priv *usp = subdev->private;
	int channel, type;

	channel = CR_CHAN(insn->chanspec);
	/* defining module type(analog or digital) */
	type = usp->usp_module_type[channel / 2];

	if (type == MODULE_DIGITAL) {
		if (!__unioxx5_digital_read(subdev, data, channel, dev->minor))
			return -1;
	} else {
		if (!__unioxx5_analog_read(subdev, data, channel, dev->minor))
			return -1;
	}

	return 1;
}

static int unioxx5_subdev_write(struct comedi_device *dev,
				struct comedi_subdevice *subdev,
				struct comedi_insn *insn, unsigned int *data)
{
	struct unioxx5_subd_priv *usp = subdev->private;
	int channel, type;

	channel = CR_CHAN(insn->chanspec);
	/* defining module type(analog or digital) */
	type = usp->usp_module_type[channel / 2];

	if (type == MODULE_DIGITAL) {
		if (!__unioxx5_digital_write(subdev, data, channel, dev->minor))
			return -1;
	} else {
		if (!__unioxx5_analog_write(subdev, data, channel, dev->minor))
			return -1;
	}

	return 1;
}

/* for digital modules only */
static int unioxx5_insn_config(struct comedi_device *dev,
			       struct comedi_subdevice *subdev,
			       struct comedi_insn *insn, unsigned int *data)
{
	int channel_offset, flags, channel = CR_CHAN(insn->chanspec), type;
	struct unioxx5_subd_priv *usp = subdev->private;
	int mask = 1 << (channel & 0x07);

	type = usp->usp_module_type[channel / 2];

	if (type != MODULE_DIGITAL) {
		dev_err(dev->class_dev,
			"channel configuration accessible only for digital modules\n");
		return -1;
	}

	channel_offset = __unioxx5_define_chan_offset(channel);
	if (channel_offset < 0) {
		dev_err(dev->class_dev,
			"undefined channel %d. channel range is 0 .. 23\n",
			channel);
		return -1;
	}

	/* gets previously written value */
	flags = usp->usp_prev_cn_val[channel_offset - 1];

	switch (*data) {
	case COMEDI_INPUT:
		flags &= ~mask;
		break;
	case COMEDI_OUTPUT:
		flags |= mask;
		break;
	default:
		dev_err(dev->class_dev, "unknown flag\n");
		return -1;
	}

	/*                                                        *\
	 * sets channels buffer to 1(after this we are allowed to *
	 * change channel type on input or output)                *
	 \*                                                        */
	outb(1, usp->usp_iobase + 0);
	/* changes type of _one_ channel */
	outb(flags, usp->usp_iobase + channel_offset);
	/* sets channels bank to 0(allows directly input/output) */
	outb(0, usp->usp_iobase + 0);
	/* saves written value */
	usp->usp_prev_cn_val[channel_offset - 1] = flags;

	return 0;
}

/* initializing subdevice with given address */
static int __unioxx5_subdev_init(struct comedi_device *dev,
				 struct comedi_subdevice *s,
				 int iobase)
{
	struct unioxx5_subd_priv *usp;
	int i, to, ndef_flag = 0;
	int ret;

	usp = comedi_alloc_spriv(s, sizeof(*usp));
	if (!usp)
		return -ENOMEM;

	ret = __comedi_request_region(dev, iobase, UNIOXX5_SIZE);
	if (ret)
		return ret;
	usp->usp_iobase = iobase;

	/* defining modules types */
	for (i = 0; i < 12; i++) {
		to = 10000;

		__unioxx5_analog_config(usp, i * 2);
		/* sends channel number to card */
		outb(i + 1, iobase + 5);
		outb('H', iobase + 6);	/* requests EEPROM world */
		while (!(inb(iobase + 0) & TxBE))
			;	/* waits while writing will be allowed */
		outb(0, iobase + 6);

		/* waits while reading of two bytes will be allowed */
		while (!(inb(iobase + 0) & Rx2CA)) {
			if (--to <= 0) {
				ndef_flag = 1;
				break;
			}
		}

		if (ndef_flag) {
			usp->usp_module_type[i] = 0;
			ndef_flag = 0;
		} else {
			usp->usp_module_type[i] = inb(iobase + 6);
		}

		udelay(1);
	}

	/* initial subdevice for digital or analog i/o */
	s->type = COMEDI_SUBD_DIO;
	s->subdev_flags = SDF_READABLE | SDF_WRITABLE;
	s->n_chan = UNIOXX5_NUM_OF_CHANS;
	s->maxdata = 0xFFF;
	s->range_table = &range_digital;
	s->insn_read = unioxx5_subdev_read;
	s->insn_write = unioxx5_subdev_write;
	/* for digital modules only!!! */
	s->insn_config = unioxx5_insn_config;

	return 0;
}

static int unioxx5_attach(struct comedi_device *dev,
			  struct comedi_devconfig *it)
{
	struct comedi_subdevice *s;
	int iobase, i, n_subd;
	int id, num, ba;
	int ret;

	iobase = it->options[0];

	dev->iobase = iobase;
	iobase += UNIOXX5_SUBDEV_BASE;
	n_subd = 0;

	/* getting number of subdevices with types 'g01' */
	for (i = 0, ba = iobase; i < 4; i++, ba += UNIOXX5_SUBDEV_ODDS) {
		id = inb(ba + 0xE);
		num = inb(ba + 0xF);

		if (id != 'g' || num != 1)
			continue;

		n_subd++;
	}

	/* unioxx5 can has from two to four subdevices */
	if (n_subd < 2) {
		dev_err(dev->class_dev,
			"your card must has at least 2 'g01' subdevices\n");
		return -1;
	}

	ret = comedi_alloc_subdevices(dev, n_subd);
	if (ret)
		return ret;

	/* initializing each of for same subdevices */
	for (i = 0; i < n_subd; i++, iobase += UNIOXX5_SUBDEV_ODDS) {
		s = &dev->subdevices[i];
		ret = __unioxx5_subdev_init(dev, s, iobase);
		if (ret)
			return ret;
	}

	return 0;
}

static void unioxx5_detach(struct comedi_device *dev)
{
	struct comedi_subdevice *s;
	struct unioxx5_subd_priv *spriv;
	int i;

	for (i = 0; i < dev->n_subdevices; i++) {
		s = &dev->subdevices[i];
		spriv = s->private;
		if (spriv && spriv->usp_iobase)
			release_region(spriv->usp_iobase, UNIOXX5_SIZE);
	}
}

static struct comedi_driver unioxx5_driver = {
	.driver_name	= "unioxx5",
	.module		= THIS_MODULE,
	.attach		= unioxx5_attach,
	.detach		= unioxx5_detach,
};
module_comedi_driver(unioxx5_driver);

MODULE_AUTHOR("Comedi http://www.comedi.org");
MODULE_DESCRIPTION("Comedi low-level driver");
MODULE_LICENSE("GPL");
