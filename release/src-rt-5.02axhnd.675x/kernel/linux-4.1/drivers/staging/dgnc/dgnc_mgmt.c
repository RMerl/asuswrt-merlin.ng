/*
 * Copyright 2003 Digi International (www.digi.com)
 *	Scott H Kilau <Scott_Kilau at digi dot com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY, EXPRESS OR IMPLIED; without even the
 * implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 * PURPOSE.  See the GNU General Public License for more details.
 */

/************************************************************************
 *
 * This file implements the mgmt functionality for the
 * Neo and ClassicBoard based product lines.
 *
 ************************************************************************
 */
#include <linux/kernel.h>
#include <linux/ctype.h>
#include <linux/sched.h>	/* For jiffies, task states */
#include <linux/interrupt.h>	/* For tasklet and interrupt structs/defines */
#include <linux/serial_reg.h>
#include <linux/termios.h>
#include <linux/uaccess.h>	/* For copy_from_user/copy_to_user */

#include "dgnc_driver.h"
#include "dgnc_pci.h"
#include "dgnc_mgmt.h"


/* Our "in use" variables, to enforce 1 open only */
static int dgnc_mgmt_in_use[MAXMGMTDEVICES];


/*
 * dgnc_mgmt_open()
 *
 * Open the mgmt/downld/dpa device
 */
int dgnc_mgmt_open(struct inode *inode, struct file *file)
{
	unsigned long flags;
	unsigned int minor = iminor(inode);

	spin_lock_irqsave(&dgnc_global_lock, flags);

	/* mgmt device */
	if (minor < MAXMGMTDEVICES) {
		/* Only allow 1 open at a time on mgmt device */
		if (dgnc_mgmt_in_use[minor]) {
			spin_unlock_irqrestore(&dgnc_global_lock, flags);
			return -EBUSY;
		}
		dgnc_mgmt_in_use[minor]++;
	} else {
		spin_unlock_irqrestore(&dgnc_global_lock, flags);
		return -ENXIO;
	}

	spin_unlock_irqrestore(&dgnc_global_lock, flags);

	return 0;
}


/*
 * dgnc_mgmt_close()
 *
 * Open the mgmt/dpa device
 */
int dgnc_mgmt_close(struct inode *inode, struct file *file)
{
	unsigned long flags;
	unsigned int minor = iminor(inode);

	spin_lock_irqsave(&dgnc_global_lock, flags);

	/* mgmt device */
	if (minor < MAXMGMTDEVICES) {
		if (dgnc_mgmt_in_use[minor])
			dgnc_mgmt_in_use[minor] = 0;
	}
	spin_unlock_irqrestore(&dgnc_global_lock, flags);

	return 0;
}


/*
 * dgnc_mgmt_ioctl()
 *
 * ioctl the mgmt/dpa device
 */

long dgnc_mgmt_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	unsigned long flags;
	void __user *uarg = (void __user *) arg;

	switch (cmd) {

	case DIGI_GETDD:
	{
		/*
		 * This returns the total number of boards
		 * in the system, as well as driver version
		 * and has space for a reserved entry
		 */
		struct digi_dinfo ddi;

		spin_lock_irqsave(&dgnc_global_lock, flags);

		ddi.dinfo_nboards = dgnc_NumBoards;
		sprintf(ddi.dinfo_version, "%s", DG_PART);

		spin_unlock_irqrestore(&dgnc_global_lock, flags);

		if (copy_to_user(uarg, &ddi, sizeof(ddi)))
			return -EFAULT;

		break;
	}

	case DIGI_GETBD:
	{
		int brd;

		struct digi_info di;

		if (copy_from_user(&brd, uarg, sizeof(int)))
			return -EFAULT;

		if (brd < 0 || brd >= dgnc_NumBoards)
			return -ENODEV;

		memset(&di, 0, sizeof(di));

		di.info_bdnum = brd;

		spin_lock_irqsave(&dgnc_Board[brd]->bd_lock, flags);

		di.info_bdtype = dgnc_Board[brd]->dpatype;
		di.info_bdstate = dgnc_Board[brd]->dpastatus;
		di.info_ioport = 0;
		di.info_physaddr = (ulong) dgnc_Board[brd]->membase;
		di.info_physsize = (ulong) dgnc_Board[brd]->membase - dgnc_Board[brd]->membase_end;
		if (dgnc_Board[brd]->state != BOARD_FAILED)
			di.info_nports = dgnc_Board[brd]->nasync;
		else
			di.info_nports = 0;

		spin_unlock_irqrestore(&dgnc_Board[brd]->bd_lock, flags);

		if (copy_to_user(uarg, &di, sizeof(di)))
			return -EFAULT;

		break;
	}

	case DIGI_GET_NI_INFO:
	{
		struct channel_t *ch;
		struct ni_info ni;
		unsigned char mstat = 0;
		uint board = 0;
		uint channel = 0;

		if (copy_from_user(&ni, uarg, sizeof(ni)))
			return -EFAULT;

		board = ni.board;
		channel = ni.channel;

		/* Verify boundaries on board */
		if (board >= dgnc_NumBoards)
			return -ENODEV;

		/* Verify boundaries on channel */
		if (channel >= dgnc_Board[board]->nasync)
			return -ENODEV;

		ch = dgnc_Board[board]->channels[channel];

		if (!ch || ch->magic != DGNC_CHANNEL_MAGIC)
			return -ENODEV;

		memset(&ni, 0, sizeof(ni));
		ni.board = board;
		ni.channel = channel;

		spin_lock_irqsave(&ch->ch_lock, flags);

		mstat = (ch->ch_mostat | ch->ch_mistat);

		if (mstat & UART_MCR_DTR) {
			ni.mstat |= TIOCM_DTR;
			ni.dtr = TIOCM_DTR;
		}
		if (mstat & UART_MCR_RTS) {
			ni.mstat |= TIOCM_RTS;
			ni.rts = TIOCM_RTS;
		}
		if (mstat & UART_MSR_CTS) {
			ni.mstat |= TIOCM_CTS;
			ni.cts = TIOCM_CTS;
		}
		if (mstat & UART_MSR_RI) {
			ni.mstat |= TIOCM_RI;
			ni.ri = TIOCM_RI;
		}
		if (mstat & UART_MSR_DCD) {
			ni.mstat |= TIOCM_CD;
			ni.dcd = TIOCM_CD;
		}
		if (mstat & UART_MSR_DSR)
			ni.mstat |= TIOCM_DSR;

		ni.iflag = ch->ch_c_iflag;
		ni.oflag = ch->ch_c_oflag;
		ni.cflag = ch->ch_c_cflag;
		ni.lflag = ch->ch_c_lflag;

		if (ch->ch_digi.digi_flags & CTSPACE ||
		    ch->ch_c_cflag & CRTSCTS)
			ni.hflow = 1;
		else
			ni.hflow = 0;

		if ((ch->ch_flags & CH_STOPI) ||
		    (ch->ch_flags & CH_FORCED_STOPI))
			ni.recv_stopped = 1;
		else
			ni.recv_stopped = 0;

		if ((ch->ch_flags & CH_STOP) || (ch->ch_flags & CH_FORCED_STOP))
			ni.xmit_stopped = 1;
		else
			ni.xmit_stopped = 0;

		ni.curtx = ch->ch_txcount;
		ni.currx = ch->ch_rxcount;

		ni.baud = ch->ch_old_baud;

		spin_unlock_irqrestore(&ch->ch_lock, flags);

		if (copy_to_user(uarg, &ni, sizeof(ni)))
			return -EFAULT;

		break;
	}


	}

	return 0;
}
