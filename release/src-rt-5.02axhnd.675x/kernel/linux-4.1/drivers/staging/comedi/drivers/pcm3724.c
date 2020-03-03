/*
    comedi/drivers/pcm724.c

    Drew Csillag <drew_csillag@yahoo.com>

    hardware driver for Advantech card:
     card:   PCM-3724
     driver: pcm3724

    Options for PCM-3724
     [0] - IO Base
*/
/*
Driver: pcm3724
Description: Advantech PCM-3724
Author: Drew Csillag <drew_csillag@yahoo.com>
Devices: [Advantech] PCM-3724 (pcm724)
Status: tested

This is driver for digital I/O boards PCM-3724 with 48 DIO.
It needs 8255.o for operations and only immediate mode is supported.
See the source for configuration details.

Copy/pasted/hacked from pcm724.c
*/
/*
 * check_driver overrides:
 *   struct comedi_insn
 */

#include <linux/module.h>
#include "../comedidev.h"

#include "8255.h"

#define BUF_C0 0x1
#define BUF_B0 0x2
#define BUF_A0 0x4
#define BUF_C1 0x8
#define BUF_B1 0x10
#define BUF_A1 0x20

#define GATE_A0 0x4
#define GATE_B0	0x2
#define GATE_C0	0x1
#define GATE_A1	0x20
#define GATE_B1	0x10
#define GATE_C1 0x8

/* used to track configured dios */
struct priv_pcm3724 {
	int dio_1;
	int dio_2;
};

static int compute_buffer(int config, int devno, struct comedi_subdevice *s)
{
	/* 1 in io_bits indicates output */
	if (s->io_bits & 0x0000ff) {
		if (devno == 0)
			config |= BUF_A0;
		else
			config |= BUF_A1;
	}
	if (s->io_bits & 0x00ff00) {
		if (devno == 0)
			config |= BUF_B0;
		else
			config |= BUF_B1;
	}
	if (s->io_bits & 0xff0000) {
		if (devno == 0)
			config |= BUF_C0;
		else
			config |= BUF_C1;
	}
	return config;
}

static void do_3724_config(struct comedi_device *dev,
			   struct comedi_subdevice *s, int chanspec)
{
	struct comedi_subdevice *s_dio1 = &dev->subdevices[0];
	struct comedi_subdevice *s_dio2 = &dev->subdevices[1];
	int config;
	int buffer_config;
	unsigned long port_8255_cfg;

	config = I8255_CTRL_CW;
	buffer_config = 0;

	/* 1 in io_bits indicates output, 1 in config indicates input */
	if (!(s->io_bits & 0x0000ff))
		config |= I8255_CTRL_A_IO;

	if (!(s->io_bits & 0x00ff00))
		config |= I8255_CTRL_B_IO;

	if (!(s->io_bits & 0xff0000))
		config |= I8255_CTRL_C_HI_IO | I8255_CTRL_C_LO_IO;

	buffer_config = compute_buffer(0, 0, s_dio1);
	buffer_config = compute_buffer(buffer_config, 1, s_dio2);

	if (s == s_dio1)
		port_8255_cfg = dev->iobase + I8255_CTRL_REG;
	else
		port_8255_cfg = dev->iobase + I8255_SIZE + I8255_CTRL_REG;

	outb(buffer_config, dev->iobase + 8);	/* update buffer register */

	outb(config, port_8255_cfg);
}

static void enable_chan(struct comedi_device *dev, struct comedi_subdevice *s,
			int chanspec)
{
	struct priv_pcm3724 *priv = dev->private;
	struct comedi_subdevice *s_dio1 = &dev->subdevices[0];
	unsigned int mask;
	int gatecfg;

	gatecfg = 0;

	mask = 1 << CR_CHAN(chanspec);
	if (s == s_dio1)
		priv->dio_1 |= mask;
	else
		priv->dio_2 |= mask;

	if (priv->dio_1 & 0xff0000)
		gatecfg |= GATE_C0;

	if (priv->dio_1 & 0xff00)
		gatecfg |= GATE_B0;

	if (priv->dio_1 & 0xff)
		gatecfg |= GATE_A0;

	if (priv->dio_2 & 0xff0000)
		gatecfg |= GATE_C1;

	if (priv->dio_2 & 0xff00)
		gatecfg |= GATE_B1;

	if (priv->dio_2 & 0xff)
		gatecfg |= GATE_A1;

	outb(gatecfg, dev->iobase + 9);
}

/* overriding the 8255 insn config */
static int subdev_3724_insn_config(struct comedi_device *dev,
				   struct comedi_subdevice *s,
				   struct comedi_insn *insn,
				   unsigned int *data)
{
	unsigned int chan = CR_CHAN(insn->chanspec);
	unsigned int mask;
	int ret;

	if (chan < 8)
		mask = 0x0000ff;
	else if (chan < 16)
		mask = 0x00ff00;
	else if (chan < 20)
		mask = 0x0f0000;
	else
		mask = 0xf00000;

	ret = comedi_dio_insn_config(dev, s, insn, data, mask);
	if (ret)
		return ret;

	do_3724_config(dev, s, insn->chanspec);
	enable_chan(dev, s, insn->chanspec);

	return insn->n;
}

static int pcm3724_attach(struct comedi_device *dev,
			  struct comedi_devconfig *it)
{
	struct priv_pcm3724 *priv;
	struct comedi_subdevice *s;
	int ret, i;

	priv = comedi_alloc_devpriv(dev, sizeof(*priv));
	if (!priv)
		return -ENOMEM;

	ret = comedi_request_region(dev, it->options[0], 0x10);
	if (ret)
		return ret;

	ret = comedi_alloc_subdevices(dev, 2);
	if (ret)
		return ret;

	for (i = 0; i < dev->n_subdevices; i++) {
		s = &dev->subdevices[i];
		ret = subdev_8255_init(dev, s, NULL, i * I8255_SIZE);
		if (ret)
			return ret;
		s->insn_config = subdev_3724_insn_config;
	}
	return 0;
}

static struct comedi_driver pcm3724_driver = {
	.driver_name	= "pcm3724",
	.module		= THIS_MODULE,
	.attach		= pcm3724_attach,
	.detach		= comedi_legacy_detach,
};
module_comedi_driver(pcm3724_driver);

MODULE_AUTHOR("Comedi http://www.comedi.org");
MODULE_DESCRIPTION("Comedi low-level driver");
MODULE_LICENSE("GPL");
