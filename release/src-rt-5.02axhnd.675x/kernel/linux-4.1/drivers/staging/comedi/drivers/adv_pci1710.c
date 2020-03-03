/*
 * comedi/drivers/adv_pci1710.c
 *
 * Author: Michal Dobes <dobes@tesnet.cz>
 *
 * Thanks to ZhenGang Shang <ZhenGang.Shang@Advantech.com.cn>
 * for testing and information.
 *
 *  hardware driver for Advantech cards:
 *   card:   PCI-1710, PCI-1710HG, PCI-1711, PCI-1713, PCI-1720, PCI-1731
 *   driver: pci1710,  pci1710hg,  pci1711,  pci1713,  pci1720,  pci1731
 *
 * Options:
 *  [0] - PCI bus number - if bus number and slot number are 0,
 *                         then driver search for first unused card
 *  [1] - PCI slot number
 *
*/
/*
Driver: adv_pci1710
Description: Advantech PCI-1710, PCI-1710HG, PCI-1711, PCI-1713,
	     Advantech PCI-1720, PCI-1731
Author: Michal Dobes <dobes@tesnet.cz>
Devices: [Advantech] PCI-1710 (adv_pci1710), PCI-1710HG (pci1710hg),
  PCI-1711 (adv_pci1710), PCI-1713, PCI-1720,
  PCI-1731
Status: works

This driver supports AI, AO, DI and DO subdevices.
AI subdevice supports cmd and insn interface,
other subdevices support only insn interface.

The PCI-1710 and PCI-1710HG have the same PCI device ID, so the
driver cannot distinguish between them, as would be normal for a
PCI driver.

Configuration options:
  [0] - PCI bus of device (optional)
  [1] - PCI slot of device (optional)
	If bus/slot is not specified, the first available PCI
	device will be used.
*/

#include <linux/module.h>
#include <linux/interrupt.h>

#include "../comedi_pci.h"

#include "comedi_8254.h"
#include "amcc_s5933.h"

#define PCI171x_AD_DATA	 0	/* R:   A/D data */
#define PCI171x_SOFTTRG	 0	/* W:   soft trigger for A/D */
#define PCI171x_RANGE	 2	/* W:   A/D gain/range register */
#define PCI171x_MUX	 4	/* W:   A/D multiplexor control */
#define PCI171x_STATUS	 6	/* R:   status register */
#define PCI171x_CONTROL	 6	/* W:   control register */
#define PCI171x_CLRINT	 8	/* W:   clear interrupts request */
#define PCI171x_CLRFIFO	 9	/* W:   clear FIFO */
#define PCI171x_DA1	10	/* W:   D/A register */
#define PCI171x_DA2	12	/* W:   D/A register */
#define PCI171x_DAREF	14	/* W:   D/A reference control */
#define PCI171x_DI	16	/* R:   digi inputs */
#define PCI171x_DO	16	/* R:   digi inputs */

#define PCI171X_TIMER_BASE	0x18

/* upper bits from status register (PCI171x_STATUS) (lower is same with control
 * reg) */
#define	Status_FE	0x0100	/* 1=FIFO is empty */
#define Status_FH	0x0200	/* 1=FIFO is half full */
#define Status_FF	0x0400	/* 1=FIFO is full, fatal error */
#define Status_IRQ	0x0800	/* 1=IRQ occurred */
/* bits from control register (PCI171x_CONTROL) */
#define Control_CNT0	0x0040	/* 1=CNT0 have external source,
				 * 0=have internal 100kHz source */
#define Control_ONEFH	0x0020	/* 1=IRQ on FIFO is half full, 0=every sample */
#define Control_IRQEN	0x0010	/* 1=enable IRQ */
#define Control_GATE	0x0008	/* 1=enable external trigger GATE (8254?) */
#define Control_EXT	0x0004	/* 1=external trigger source */
#define Control_PACER	0x0002	/* 1=enable internal 8254 trigger source */
#define Control_SW	0x0001	/* 1=enable software trigger source */

#define PCI1720_DA0	 0	/* W:   D/A register 0 */
#define PCI1720_DA1	 2	/* W:   D/A register 1 */
#define PCI1720_DA2	 4	/* W:   D/A register 2 */
#define PCI1720_DA3	 6	/* W:   D/A register 3 */
#define PCI1720_RANGE	 8	/* R/W: D/A range register */
#define PCI1720_SYNCOUT	 9	/* W:   D/A synchronized output register */
#define PCI1720_SYNCONT	15	/* R/W: D/A synchronized control */

/* D/A synchronized control (PCI1720_SYNCONT) */
#define Syncont_SC0	 1	/* set synchronous output mode */

static const struct comedi_lrange range_pci1710_3 = {
	9, {
		BIP_RANGE(5),
		BIP_RANGE(2.5),
		BIP_RANGE(1.25),
		BIP_RANGE(0.625),
		BIP_RANGE(10),
		UNI_RANGE(10),
		UNI_RANGE(5),
		UNI_RANGE(2.5),
		UNI_RANGE(1.25)
	}
};

static const char range_codes_pci1710_3[] = { 0x00, 0x01, 0x02, 0x03, 0x04,
					      0x10, 0x11, 0x12, 0x13 };

static const struct comedi_lrange range_pci1710hg = {
	12, {
		BIP_RANGE(5),
		BIP_RANGE(0.5),
		BIP_RANGE(0.05),
		BIP_RANGE(0.005),
		BIP_RANGE(10),
		BIP_RANGE(1),
		BIP_RANGE(0.1),
		BIP_RANGE(0.01),
		UNI_RANGE(10),
		UNI_RANGE(1),
		UNI_RANGE(0.1),
		UNI_RANGE(0.01)
	}
};

static const char range_codes_pci1710hg[] = { 0x00, 0x01, 0x02, 0x03, 0x04,
					      0x05, 0x06, 0x07, 0x10, 0x11,
					      0x12, 0x13 };

static const struct comedi_lrange range_pci17x1 = {
	5, {
		BIP_RANGE(10),
		BIP_RANGE(5),
		BIP_RANGE(2.5),
		BIP_RANGE(1.25),
		BIP_RANGE(0.625)
	}
};

static const char range_codes_pci17x1[] = { 0x00, 0x01, 0x02, 0x03, 0x04 };

static const struct comedi_lrange pci1720_ao_range = {
	4, {
		UNI_RANGE(5),
		UNI_RANGE(10),
		BIP_RANGE(5),
		BIP_RANGE(10)
	}
};

static const struct comedi_lrange pci171x_ao_range = {
	2, {
		UNI_RANGE(5),
		UNI_RANGE(10)
	}
};

enum pci1710_boardid {
	BOARD_PCI1710,
	BOARD_PCI1710HG,
	BOARD_PCI1711,
	BOARD_PCI1713,
	BOARD_PCI1720,
	BOARD_PCI1731,
};

struct boardtype {
	const char *name;	/*  board name */
	int n_aichan;		/*  num of A/D chans */
	const struct comedi_lrange *rangelist_ai;	/*  rangelist for A/D */
	const char *rangecode_ai;	/*  range codes for programming */
	unsigned int is_pci1713:1;
	unsigned int is_pci1720:1;
	unsigned int has_irq:1;
	unsigned int has_large_fifo:1;	/* 4K or 1K FIFO */
	unsigned int has_diff_ai:1;
	unsigned int has_ao:1;
	unsigned int has_di_do:1;
	unsigned int has_counter:1;
};

static const struct boardtype boardtypes[] = {
	[BOARD_PCI1710] = {
		.name		= "pci1710",
		.n_aichan	= 16,
		.rangelist_ai	= &range_pci1710_3,
		.rangecode_ai	= range_codes_pci1710_3,
		.has_irq	= 1,
		.has_large_fifo	= 1,
		.has_diff_ai	= 1,
		.has_ao		= 1,
		.has_di_do	= 1,
		.has_counter	= 1,
	},
	[BOARD_PCI1710HG] = {
		.name		= "pci1710hg",
		.n_aichan	= 16,
		.rangelist_ai	= &range_pci1710hg,
		.rangecode_ai	= range_codes_pci1710hg,
		.has_irq	= 1,
		.has_large_fifo	= 1,
		.has_diff_ai	= 1,
		.has_ao		= 1,
		.has_di_do	= 1,
		.has_counter	= 1,
	},
	[BOARD_PCI1711] = {
		.name		= "pci1711",
		.n_aichan	= 16,
		.rangelist_ai	= &range_pci17x1,
		.rangecode_ai	= range_codes_pci17x1,
		.has_irq	= 1,
		.has_ao		= 1,
		.has_di_do	= 1,
		.has_counter	= 1,
	},
	[BOARD_PCI1713] = {
		.name		= "pci1713",
		.n_aichan	= 32,
		.rangelist_ai	= &range_pci1710_3,
		.rangecode_ai	= range_codes_pci1710_3,
		.is_pci1713	= 1,
		.has_irq	= 1,
		.has_large_fifo	= 1,
		.has_diff_ai	= 1,
	},
	[BOARD_PCI1720] = {
		.name		= "pci1720",
		.is_pci1720	= 1,
		.has_ao		= 1,
	},
	[BOARD_PCI1731] = {
		.name		= "pci1731",
		.n_aichan	= 16,
		.rangelist_ai	= &range_pci17x1,
		.rangecode_ai	= range_codes_pci17x1,
		.has_irq	= 1,
		.has_di_do	= 1,
	},
};

struct pci1710_private {
	unsigned int max_samples;
	unsigned int CntrlReg;	/*  Control register */
	unsigned char ai_et;
	unsigned int ai_et_CntrlReg;
	unsigned int ai_et_MuxVal;
	unsigned int act_chanlist[32];	/*  list of scanned channel */
	unsigned char saved_seglen;	/* len of the non-repeating chanlist */
	unsigned char da_ranges;	/*  copy of D/A outpit range register */
};

static int pci171x_ai_check_chanlist(struct comedi_device *dev,
				     struct comedi_subdevice *s,
				     struct comedi_cmd *cmd)
{
	struct pci1710_private *devpriv = dev->private;
	unsigned int chan0 = CR_CHAN(cmd->chanlist[0]);
	unsigned int last_aref = CR_AREF(cmd->chanlist[0]);
	unsigned int next_chan = (chan0 + 1) % s->n_chan;
	unsigned int chansegment[32];
	unsigned int seglen;
	int i;

	if (cmd->chanlist_len == 1) {
		devpriv->saved_seglen = cmd->chanlist_len;
		return 0;
	}

	/* first channel is always ok */
	chansegment[0] = cmd->chanlist[0];

	for (i = 1; i < cmd->chanlist_len; i++) {
		unsigned int chan = CR_CHAN(cmd->chanlist[i]);
		unsigned int aref = CR_AREF(cmd->chanlist[i]);

		if (cmd->chanlist[0] == cmd->chanlist[i])
			break;	/*  we detected a loop, stop */

		if (aref == AREF_DIFF && (chan & 1)) {
			dev_err(dev->class_dev,
				"Odd channel cannot be differential input!\n");
			return -EINVAL;
		}

		if (last_aref == AREF_DIFF)
			next_chan = (next_chan + 1) % s->n_chan;
		if (chan != next_chan) {
			dev_err(dev->class_dev,
				"channel list must be continuous! chanlist[%i]=%d but must be %d or %d!\n",
				i, chan, next_chan, chan0);
			return -EINVAL;
		}

		/* next correct channel in list */
		chansegment[i] = cmd->chanlist[i];
		last_aref = aref;
	}
	seglen = i;

	for (i = 0; i < cmd->chanlist_len; i++) {
		if (cmd->chanlist[i] != chansegment[i % seglen]) {
			dev_err(dev->class_dev,
				"bad channel, reference or range number! chanlist[%i]=%d,%d,%d and not %d,%d,%d!\n",
				i, CR_CHAN(chansegment[i]),
				CR_RANGE(chansegment[i]),
				CR_AREF(chansegment[i]),
				CR_CHAN(cmd->chanlist[i % seglen]),
				CR_RANGE(cmd->chanlist[i % seglen]),
				CR_AREF(chansegment[i % seglen]));
			return -EINVAL;
		}
	}
	devpriv->saved_seglen = seglen;

	return 0;
}

static void pci171x_ai_setup_chanlist(struct comedi_device *dev,
				      struct comedi_subdevice *s,
				      unsigned int *chanlist,
				      unsigned int n_chan,
				      unsigned int seglen)
{
	const struct boardtype *board = dev->board_ptr;
	struct pci1710_private *devpriv = dev->private;
	unsigned int first_chan = CR_CHAN(chanlist[0]);
	unsigned int last_chan = CR_CHAN(chanlist[seglen - 1]);
	unsigned int i;

	for (i = 0; i < seglen; i++) {	/*  store range list to card */
		unsigned int chan = CR_CHAN(chanlist[i]);
		unsigned int range = CR_RANGE(chanlist[i]);
		unsigned int aref = CR_AREF(chanlist[i]);
		unsigned int rangeval;

		rangeval = board->rangecode_ai[range];
		if (aref == AREF_DIFF)
			rangeval |= 0x0020;

		/* select channel and set range */
		outw(chan | (chan << 8), dev->iobase + PCI171x_MUX);
		outw(rangeval, dev->iobase + PCI171x_RANGE);

		devpriv->act_chanlist[i] = chan;
	}
	for ( ; i < n_chan; i++)	/* store remainder of channel list */
		devpriv->act_chanlist[i] = CR_CHAN(chanlist[i]);

	/* select channel interval to scan */
	devpriv->ai_et_MuxVal = first_chan | (last_chan << 8);
	outw(devpriv->ai_et_MuxVal, dev->iobase + PCI171x_MUX);
}

static int pci171x_ai_eoc(struct comedi_device *dev,
			  struct comedi_subdevice *s,
			  struct comedi_insn *insn,
			  unsigned long context)
{
	unsigned int status;

	status = inw(dev->iobase + PCI171x_STATUS);
	if ((status & Status_FE) == 0)
		return 0;
	return -EBUSY;
}

static int pci171x_ai_read_sample(struct comedi_device *dev,
				  struct comedi_subdevice *s,
				  unsigned int cur_chan,
				  unsigned int *val)
{
	const struct boardtype *board = dev->board_ptr;
	struct pci1710_private *devpriv = dev->private;
	unsigned int sample;
	unsigned int chan;

	sample = inw(dev->iobase + PCI171x_AD_DATA);
	if (!board->is_pci1713) {
		/*
		 * The upper 4 bits of the 16-bit sample are the channel number
		 * that the sample was acquired from. Verify that this channel
		 * number matches the expected channel number.
		 */
		chan = sample >> 12;
		if (chan != devpriv->act_chanlist[cur_chan]) {
			dev_err(dev->class_dev,
				"A/D data droput: received from channel %d, expected %d\n",
				chan, devpriv->act_chanlist[cur_chan]);
			return -ENODATA;
		}
	}
	*val = sample & s->maxdata;
	return 0;
}

static int pci171x_ai_insn_read(struct comedi_device *dev,
				struct comedi_subdevice *s,
				struct comedi_insn *insn,
				unsigned int *data)
{
	struct pci1710_private *devpriv = dev->private;
	int ret = 0;
	int i;

	devpriv->CntrlReg &= Control_CNT0;
	devpriv->CntrlReg |= Control_SW;	/*  set software trigger */
	outw(devpriv->CntrlReg, dev->iobase + PCI171x_CONTROL);
	outb(0, dev->iobase + PCI171x_CLRFIFO);
	outb(0, dev->iobase + PCI171x_CLRINT);

	pci171x_ai_setup_chanlist(dev, s, &insn->chanspec, 1, 1);

	for (i = 0; i < insn->n; i++) {
		unsigned int val;

		outw(0, dev->iobase + PCI171x_SOFTTRG);	/* start conversion */

		ret = comedi_timeout(dev, s, insn, pci171x_ai_eoc, 0);
		if (ret)
			break;

		ret = pci171x_ai_read_sample(dev, s, 0, &val);
		if (ret)
			break;

		data[i] = val;
	}

	outb(0, dev->iobase + PCI171x_CLRFIFO);
	outb(0, dev->iobase + PCI171x_CLRINT);

	return ret ? ret : insn->n;
}

static int pci171x_ao_insn_write(struct comedi_device *dev,
				 struct comedi_subdevice *s,
				 struct comedi_insn *insn,
				 unsigned int *data)
{
	struct pci1710_private *devpriv = dev->private;
	unsigned int chan = CR_CHAN(insn->chanspec);
	unsigned int range = CR_RANGE(insn->chanspec);
	unsigned int reg = chan ? PCI171x_DA2 : PCI171x_DA1;
	unsigned int val = s->readback[chan];
	int i;

	devpriv->da_ranges &= ~(1 << (chan << 1));
	devpriv->da_ranges |= (range << (chan << 1));
	outw(devpriv->da_ranges, dev->iobase + PCI171x_DAREF);

	for (i = 0; i < insn->n; i++) {
		val = data[i];
		outw(val, dev->iobase + reg);
	}

	s->readback[chan] = val;

	return insn->n;
}

static int pci171x_di_insn_bits(struct comedi_device *dev,
				struct comedi_subdevice *s,
				struct comedi_insn *insn,
				unsigned int *data)
{
	data[1] = inw(dev->iobase + PCI171x_DI);

	return insn->n;
}

static int pci171x_do_insn_bits(struct comedi_device *dev,
				struct comedi_subdevice *s,
				struct comedi_insn *insn,
				unsigned int *data)
{
	if (comedi_dio_update_state(s, data))
		outw(s->state, dev->iobase + PCI171x_DO);

	data[1] = s->state;

	return insn->n;
}

static int pci1720_ao_insn_write(struct comedi_device *dev,
				 struct comedi_subdevice *s,
				 struct comedi_insn *insn,
				 unsigned int *data)
{
	struct pci1710_private *devpriv = dev->private;
	unsigned int chan = CR_CHAN(insn->chanspec);
	unsigned int range = CR_RANGE(insn->chanspec);
	unsigned int val;
	int i;

	val = devpriv->da_ranges & (~(0x03 << (chan << 1)));
	val |= (range << (chan << 1));
	if (val != devpriv->da_ranges) {
		outb(val, dev->iobase + PCI1720_RANGE);
		devpriv->da_ranges = val;
	}

	val = s->readback[chan];
	for (i = 0; i < insn->n; i++) {
		val = data[i];
		outw(val, dev->iobase + PCI1720_DA0 + (chan << 1));
		outb(0, dev->iobase + PCI1720_SYNCOUT);	/* update outputs */
	}

	s->readback[chan] = val;

	return insn->n;
}

static int pci171x_ai_cancel(struct comedi_device *dev,
			     struct comedi_subdevice *s)
{
	struct pci1710_private *devpriv = dev->private;

	devpriv->CntrlReg &= Control_CNT0;
	devpriv->CntrlReg |= Control_SW;
	/* reset any operations */
	outw(devpriv->CntrlReg, dev->iobase + PCI171x_CONTROL);
	comedi_8254_pacer_enable(dev->pacer, 1, 2, false);
	outb(0, dev->iobase + PCI171x_CLRFIFO);
	outb(0, dev->iobase + PCI171x_CLRINT);

	return 0;
}

static void pci1710_handle_every_sample(struct comedi_device *dev,
					struct comedi_subdevice *s)
{
	struct comedi_cmd *cmd = &s->async->cmd;
	unsigned int status;
	unsigned int val;
	int ret;

	status = inw(dev->iobase + PCI171x_STATUS);
	if (status & Status_FE) {
		dev_dbg(dev->class_dev, "A/D FIFO empty (%4x)\n", status);
		s->async->events |= COMEDI_CB_ERROR;
		return;
	}
	if (status & Status_FF) {
		dev_dbg(dev->class_dev,
			"A/D FIFO Full status (Fatal Error!) (%4x)\n", status);
		s->async->events |= COMEDI_CB_ERROR;
		return;
	}

	outb(0, dev->iobase + PCI171x_CLRINT);	/*  clear our INT request */

	for (; !(inw(dev->iobase + PCI171x_STATUS) & Status_FE);) {
		ret = pci171x_ai_read_sample(dev, s, s->async->cur_chan, &val);
		if (ret) {
			s->async->events |= COMEDI_CB_ERROR;
			break;
		}

		comedi_buf_write_samples(s, &val, 1);

		if (cmd->stop_src == TRIG_COUNT &&
		    s->async->scans_done >= cmd->stop_arg) {
			s->async->events |= COMEDI_CB_EOA;
			break;
		}
	}

	outb(0, dev->iobase + PCI171x_CLRINT);	/*  clear our INT request */
}

static void pci1710_handle_fifo(struct comedi_device *dev,
				struct comedi_subdevice *s)
{
	struct pci1710_private *devpriv = dev->private;
	struct comedi_async *async = s->async;
	struct comedi_cmd *cmd = &async->cmd;
	unsigned int status;
	int i;

	status = inw(dev->iobase + PCI171x_STATUS);
	if (!(status & Status_FH)) {
		dev_dbg(dev->class_dev, "A/D FIFO not half full!\n");
		async->events |= COMEDI_CB_ERROR;
		return;
	}
	if (status & Status_FF) {
		dev_dbg(dev->class_dev,
			"A/D FIFO Full status (Fatal Error!)\n");
		async->events |= COMEDI_CB_ERROR;
		return;
	}

	for (i = 0; i < devpriv->max_samples; i++) {
		unsigned int val;
		int ret;

		ret = pci171x_ai_read_sample(dev, s, s->async->cur_chan, &val);
		if (ret) {
			s->async->events |= COMEDI_CB_ERROR;
			break;
		}

		if (!comedi_buf_write_samples(s, &val, 1))
			break;

		if (cmd->stop_src == TRIG_COUNT &&
		    async->scans_done >= cmd->stop_arg) {
			async->events |= COMEDI_CB_EOA;
			break;
		}
	}

	outb(0, dev->iobase + PCI171x_CLRINT);	/*  clear our INT request */
}

static irqreturn_t interrupt_service_pci1710(int irq, void *d)
{
	struct comedi_device *dev = d;
	struct pci1710_private *devpriv = dev->private;
	struct comedi_subdevice *s;
	struct comedi_cmd *cmd;

	if (!dev->attached)	/*  is device attached? */
		return IRQ_NONE;	/*  no, exit */

	s = dev->read_subdev;
	cmd = &s->async->cmd;

	/*  is this interrupt from our board? */
	if (!(inw(dev->iobase + PCI171x_STATUS) & Status_IRQ))
		return IRQ_NONE;	/*  no, exit */

	if (devpriv->ai_et) {	/*  Switch from initial TRIG_EXT to TRIG_xxx. */
		devpriv->ai_et = 0;
		devpriv->CntrlReg &= Control_CNT0;
		devpriv->CntrlReg |= Control_SW; /* set software trigger */
		outw(devpriv->CntrlReg, dev->iobase + PCI171x_CONTROL);
		devpriv->CntrlReg = devpriv->ai_et_CntrlReg;
		outb(0, dev->iobase + PCI171x_CLRFIFO);
		outb(0, dev->iobase + PCI171x_CLRINT);
		outw(devpriv->ai_et_MuxVal, dev->iobase + PCI171x_MUX);
		outw(devpriv->CntrlReg, dev->iobase + PCI171x_CONTROL);
		comedi_8254_pacer_enable(dev->pacer, 1, 2, true);
		return IRQ_HANDLED;
	}

	if (cmd->flags & CMDF_WAKE_EOS)
		pci1710_handle_every_sample(dev, s);
	else
		pci1710_handle_fifo(dev, s);

	comedi_handle_events(dev, s);

	return IRQ_HANDLED;
}

static int pci171x_ai_cmd(struct comedi_device *dev, struct comedi_subdevice *s)
{
	struct pci1710_private *devpriv = dev->private;
	struct comedi_cmd *cmd = &s->async->cmd;

	pci171x_ai_setup_chanlist(dev, s, cmd->chanlist, cmd->chanlist_len,
				  devpriv->saved_seglen);

	outb(0, dev->iobase + PCI171x_CLRFIFO);
	outb(0, dev->iobase + PCI171x_CLRINT);

	devpriv->CntrlReg &= Control_CNT0;
	if ((cmd->flags & CMDF_WAKE_EOS) == 0)
		devpriv->CntrlReg |= Control_ONEFH;

	if (cmd->convert_src == TRIG_TIMER) {
		comedi_8254_update_divisors(dev->pacer);

		devpriv->CntrlReg |= Control_PACER | Control_IRQEN;
		if (cmd->start_src == TRIG_EXT) {
			devpriv->ai_et_CntrlReg = devpriv->CntrlReg;
			devpriv->CntrlReg &=
			    ~(Control_PACER | Control_ONEFH | Control_GATE);
			devpriv->CntrlReg |= Control_EXT;
			devpriv->ai_et = 1;
		} else {	/* TRIG_NOW */
			devpriv->ai_et = 0;
		}
		outw(devpriv->CntrlReg, dev->iobase + PCI171x_CONTROL);

		if (cmd->start_src == TRIG_NOW)
			comedi_8254_pacer_enable(dev->pacer, 1, 2, true);
	} else {	/* TRIG_EXT */
		devpriv->CntrlReg |= Control_EXT | Control_IRQEN;
		outw(devpriv->CntrlReg, dev->iobase + PCI171x_CONTROL);
	}

	return 0;
}

static int pci171x_ai_cmdtest(struct comedi_device *dev,
			      struct comedi_subdevice *s,
			      struct comedi_cmd *cmd)
{
	int err = 0;

	/* Step 1 : check if triggers are trivially valid */

	err |= comedi_check_trigger_src(&cmd->start_src, TRIG_NOW | TRIG_EXT);
	err |= comedi_check_trigger_src(&cmd->scan_begin_src, TRIG_FOLLOW);
	err |= comedi_check_trigger_src(&cmd->convert_src,
					TRIG_TIMER | TRIG_EXT);
	err |= comedi_check_trigger_src(&cmd->scan_end_src, TRIG_COUNT);
	err |= comedi_check_trigger_src(&cmd->stop_src, TRIG_COUNT | TRIG_NONE);

	if (err)
		return 1;

	/* step 2a: make sure trigger sources are unique */

	err |= comedi_check_trigger_is_unique(cmd->start_src);
	err |= comedi_check_trigger_is_unique(cmd->convert_src);
	err |= comedi_check_trigger_is_unique(cmd->stop_src);

	/* step 2b: and mutually compatible */

	if (err)
		return 2;

	/* Step 3: check if arguments are trivially valid */

	err |= comedi_check_trigger_arg_is(&cmd->start_arg, 0);
	err |= comedi_check_trigger_arg_is(&cmd->scan_begin_arg, 0);

	if (cmd->convert_src == TRIG_TIMER)
		err |= comedi_check_trigger_arg_min(&cmd->convert_arg, 10000);
	else	/* TRIG_FOLLOW */
		err |= comedi_check_trigger_arg_is(&cmd->convert_arg, 0);

	err |= comedi_check_trigger_arg_is(&cmd->scan_end_arg,
					   cmd->chanlist_len);

	if (cmd->stop_src == TRIG_COUNT)
		err |= comedi_check_trigger_arg_min(&cmd->stop_arg, 1);
	else	/* TRIG_NONE */
		err |= comedi_check_trigger_arg_is(&cmd->stop_arg, 0);

	if (err)
		return 3;

	/* step 4: fix up any arguments */

	if (cmd->convert_src == TRIG_TIMER) {
		unsigned int arg = cmd->convert_arg;

		comedi_8254_cascade_ns_to_timer(dev->pacer, &arg, cmd->flags);
		err |= comedi_check_trigger_arg_is(&cmd->convert_arg, arg);
	}

	if (err)
		return 4;

	/* Step 5: check channel list */

	err |= pci171x_ai_check_chanlist(dev, s, cmd);

	if (err)
		return 5;

	return 0;
}

static int pci171x_insn_counter_config(struct comedi_device *dev,
				       struct comedi_subdevice *s,
				       struct comedi_insn *insn,
				       unsigned int *data)
{
	struct pci1710_private *devpriv = dev->private;

	switch (data[0]) {
	case INSN_CONFIG_SET_CLOCK_SRC:
		switch (data[1]) {
		case 0:	/* internal */
			devpriv->ai_et_CntrlReg &= ~Control_CNT0;
			break;
		case 1:	/* external */
			devpriv->ai_et_CntrlReg |= Control_CNT0;
			break;
		default:
			return -EINVAL;
		}
		outw(devpriv->ai_et_CntrlReg, dev->iobase + PCI171x_CONTROL);
		break;
	case INSN_CONFIG_GET_CLOCK_SRC:
		if (devpriv->ai_et_CntrlReg & Control_CNT0) {
			data[1] = 1;
			data[2] = 0;
		} else {
			data[1] = 0;
			data[2] = I8254_OSC_BASE_10MHZ;
		}
		break;
	default:
		return -EINVAL;
	}

	return insn->n;
}

static int pci171x_reset(struct comedi_device *dev)
{
	const struct boardtype *board = dev->board_ptr;
	struct pci1710_private *devpriv = dev->private;

	/* Software trigger, CNT0=external */
	devpriv->CntrlReg = Control_SW | Control_CNT0;
	/* reset any operations */
	outw(devpriv->CntrlReg, dev->iobase + PCI171x_CONTROL);
	outb(0, dev->iobase + PCI171x_CLRFIFO);	/*  clear FIFO */
	outb(0, dev->iobase + PCI171x_CLRINT);	/*  clear INT request */
	devpriv->da_ranges = 0;
	if (board->has_ao) {
		/* set DACs to 0..5V */
		outb(devpriv->da_ranges, dev->iobase + PCI171x_DAREF);
		outw(0, dev->iobase + PCI171x_DA1); /* set DA outputs to 0V */
		outw(0, dev->iobase + PCI171x_DA2);
	}
	outw(0, dev->iobase + PCI171x_DO);	/*  digital outputs to 0 */
	outb(0, dev->iobase + PCI171x_CLRFIFO);	/*  clear FIFO */
	outb(0, dev->iobase + PCI171x_CLRINT);	/*  clear INT request */

	return 0;
}

static int pci1720_reset(struct comedi_device *dev)
{
	struct pci1710_private *devpriv = dev->private;
	/* set synchronous output mode */
	outb(Syncont_SC0, dev->iobase + PCI1720_SYNCONT);
	devpriv->da_ranges = 0xAA;
	/* set all ranges to +/-5V */
	outb(devpriv->da_ranges, dev->iobase + PCI1720_RANGE);
	outw(0x0800, dev->iobase + PCI1720_DA0);	/*  set outputs to 0V */
	outw(0x0800, dev->iobase + PCI1720_DA1);
	outw(0x0800, dev->iobase + PCI1720_DA2);
	outw(0x0800, dev->iobase + PCI1720_DA3);
	outb(0, dev->iobase + PCI1720_SYNCOUT);	/*  update outputs */

	return 0;
}

static int pci1710_reset(struct comedi_device *dev)
{
	const struct boardtype *board = dev->board_ptr;

	if (board->is_pci1720)
		return pci1720_reset(dev);

	return pci171x_reset(dev);
}

static int pci1710_auto_attach(struct comedi_device *dev,
			       unsigned long context)
{
	struct pci_dev *pcidev = comedi_to_pci_dev(dev);
	const struct boardtype *board = NULL;
	struct pci1710_private *devpriv;
	struct comedi_subdevice *s;
	int ret, subdev, n_subdevices;

	if (context < ARRAY_SIZE(boardtypes))
		board = &boardtypes[context];
	if (!board)
		return -ENODEV;
	dev->board_ptr = board;
	dev->board_name = board->name;

	devpriv = comedi_alloc_devpriv(dev, sizeof(*devpriv));
	if (!devpriv)
		return -ENOMEM;

	ret = comedi_pci_enable(dev);
	if (ret)
		return ret;
	dev->iobase = pci_resource_start(pcidev, 2);

	dev->pacer = comedi_8254_init(dev->iobase + PCI171X_TIMER_BASE,
				      I8254_OSC_BASE_10MHZ, I8254_IO16, 0);
	if (!dev->pacer)
		return -ENOMEM;

	n_subdevices = 0;
	if (board->n_aichan)
		n_subdevices++;
	if (board->has_ao)
		n_subdevices++;
	if (board->has_di_do)
		n_subdevices += 2;
	if (board->has_counter)
		n_subdevices++;

	ret = comedi_alloc_subdevices(dev, n_subdevices);
	if (ret)
		return ret;

	pci1710_reset(dev);

	if (board->has_irq && pcidev->irq) {
		ret = request_irq(pcidev->irq, interrupt_service_pci1710,
				  IRQF_SHARED, dev->board_name, dev);
		if (ret == 0)
			dev->irq = pcidev->irq;
	}

	subdev = 0;

	if (board->n_aichan) {
		s = &dev->subdevices[subdev];
		s->type		= COMEDI_SUBD_AI;
		s->subdev_flags	= SDF_READABLE | SDF_COMMON | SDF_GROUND;
		if (board->has_diff_ai)
			s->subdev_flags	|= SDF_DIFF;
		s->n_chan	= board->n_aichan;
		s->maxdata	= 0x0fff;
		s->range_table	= board->rangelist_ai;
		s->insn_read	= pci171x_ai_insn_read;
		if (dev->irq) {
			dev->read_subdev = s;
			s->subdev_flags	|= SDF_CMD_READ;
			s->len_chanlist	= s->n_chan;
			s->do_cmdtest	= pci171x_ai_cmdtest;
			s->do_cmd	= pci171x_ai_cmd;
			s->cancel	= pci171x_ai_cancel;
		}
		subdev++;
	}

	if (board->has_ao) {
		s = &dev->subdevices[subdev];
		s->type		= COMEDI_SUBD_AO;
		s->subdev_flags	= SDF_WRITABLE | SDF_GROUND | SDF_COMMON;
		s->maxdata	= 0x0fff;
		if (board->is_pci1720) {
			s->n_chan	= 4;
			s->range_table	= &pci1720_ao_range;
			s->insn_write	= pci1720_ao_insn_write;
		} else {
			s->n_chan	= 2;
			s->range_table	= &pci171x_ao_range;
			s->insn_write	= pci171x_ao_insn_write;
		}

		ret = comedi_alloc_subdev_readback(s);
		if (ret)
			return ret;

		/* initialize the readback values to match the board reset */
		if (board->is_pci1720) {
			int i;

			for (i = 0; i < s->n_chan; i++)
				s->readback[i] = 0x0800;
		}

		subdev++;
	}

	if (board->has_di_do) {
		s = &dev->subdevices[subdev];
		s->type		= COMEDI_SUBD_DI;
		s->subdev_flags	= SDF_READABLE;
		s->n_chan	= 16;
		s->maxdata	= 1;
		s->range_table	= &range_digital;
		s->insn_bits	= pci171x_di_insn_bits;
		subdev++;

		s = &dev->subdevices[subdev];
		s->type		= COMEDI_SUBD_DO;
		s->subdev_flags	= SDF_WRITABLE;
		s->n_chan	= 16;
		s->maxdata	= 1;
		s->range_table	= &range_digital;
		s->insn_bits	= pci171x_do_insn_bits;
		subdev++;
	}

	/* Counter subdevice (8254) */
	if (board->has_counter) {
		s = &dev->subdevices[subdev];
		comedi_8254_subdevice_init(s, dev->pacer);

		dev->pacer->insn_config = pci171x_insn_counter_config;

		/* counters 1 and 2 are used internally for the pacer */
		comedi_8254_set_busy(dev->pacer, 1, true);
		comedi_8254_set_busy(dev->pacer, 2, true);

		subdev++;
	}

	/* max_samples is half the FIFO size (2 bytes/sample) */
	devpriv->max_samples = (board->has_large_fifo) ? 2048 : 512;

	return 0;
}

static void pci1710_detach(struct comedi_device *dev)
{
	if (dev->iobase)
		pci1710_reset(dev);
	comedi_pci_detach(dev);
}

static struct comedi_driver adv_pci1710_driver = {
	.driver_name	= "adv_pci1710",
	.module		= THIS_MODULE,
	.auto_attach	= pci1710_auto_attach,
	.detach		= pci1710_detach,
};

static int adv_pci1710_pci_probe(struct pci_dev *dev,
				 const struct pci_device_id *id)
{
	return comedi_pci_auto_config(dev, &adv_pci1710_driver,
				      id->driver_data);
}

static const struct pci_device_id adv_pci1710_pci_table[] = {
	{
		PCI_DEVICE_SUB(PCI_VENDOR_ID_ADVANTECH, 0x1710,
			       PCI_VENDOR_ID_PLX, PCI_DEVICE_ID_PLX_9050),
		.driver_data = BOARD_PCI1710,
	}, {
		PCI_DEVICE_SUB(PCI_VENDOR_ID_ADVANTECH, 0x1710,
			       PCI_VENDOR_ID_ADVANTECH, 0x0000),
		.driver_data = BOARD_PCI1710,
	}, {
		PCI_DEVICE_SUB(PCI_VENDOR_ID_ADVANTECH, 0x1710,
			       PCI_VENDOR_ID_ADVANTECH, 0xb100),
		.driver_data = BOARD_PCI1710,
	}, {
		PCI_DEVICE_SUB(PCI_VENDOR_ID_ADVANTECH, 0x1710,
			       PCI_VENDOR_ID_ADVANTECH, 0xb200),
		.driver_data = BOARD_PCI1710,
	}, {
		PCI_DEVICE_SUB(PCI_VENDOR_ID_ADVANTECH, 0x1710,
			       PCI_VENDOR_ID_ADVANTECH, 0xc100),
		.driver_data = BOARD_PCI1710,
	}, {
		PCI_DEVICE_SUB(PCI_VENDOR_ID_ADVANTECH, 0x1710,
			       PCI_VENDOR_ID_ADVANTECH, 0xc200),
		.driver_data = BOARD_PCI1710,
	}, {
		PCI_DEVICE_SUB(PCI_VENDOR_ID_ADVANTECH, 0x1710, 0x1000, 0xd100),
		.driver_data = BOARD_PCI1710,
	}, {
		PCI_DEVICE_SUB(PCI_VENDOR_ID_ADVANTECH, 0x1710,
			       PCI_VENDOR_ID_ADVANTECH, 0x0002),
		.driver_data = BOARD_PCI1710HG,
	}, {
		PCI_DEVICE_SUB(PCI_VENDOR_ID_ADVANTECH, 0x1710,
			       PCI_VENDOR_ID_ADVANTECH, 0xb102),
		.driver_data = BOARD_PCI1710HG,
	}, {
		PCI_DEVICE_SUB(PCI_VENDOR_ID_ADVANTECH, 0x1710,
			       PCI_VENDOR_ID_ADVANTECH, 0xb202),
		.driver_data = BOARD_PCI1710HG,
	}, {
		PCI_DEVICE_SUB(PCI_VENDOR_ID_ADVANTECH, 0x1710,
			       PCI_VENDOR_ID_ADVANTECH, 0xc102),
		.driver_data = BOARD_PCI1710HG,
	}, {
		PCI_DEVICE_SUB(PCI_VENDOR_ID_ADVANTECH, 0x1710,
			       PCI_VENDOR_ID_ADVANTECH, 0xc202),
		.driver_data = BOARD_PCI1710HG,
	}, {
		PCI_DEVICE_SUB(PCI_VENDOR_ID_ADVANTECH, 0x1710, 0x1000, 0xd102),
		.driver_data = BOARD_PCI1710HG,
	},
	{ PCI_VDEVICE(ADVANTECH, 0x1711), BOARD_PCI1711 },
	{ PCI_VDEVICE(ADVANTECH, 0x1713), BOARD_PCI1713 },
	{ PCI_VDEVICE(ADVANTECH, 0x1720), BOARD_PCI1720 },
	{ PCI_VDEVICE(ADVANTECH, 0x1731), BOARD_PCI1731 },
	{ 0 }
};
MODULE_DEVICE_TABLE(pci, adv_pci1710_pci_table);

static struct pci_driver adv_pci1710_pci_driver = {
	.name		= "adv_pci1710",
	.id_table	= adv_pci1710_pci_table,
	.probe		= adv_pci1710_pci_probe,
	.remove		= comedi_pci_auto_unconfig,
};
module_comedi_pci_driver(adv_pci1710_driver, adv_pci1710_pci_driver);

MODULE_AUTHOR("Comedi http://www.comedi.org");
MODULE_DESCRIPTION("Comedi: Advantech PCI-1710 Series Multifunction DAS Cards");
MODULE_LICENSE("GPL");
