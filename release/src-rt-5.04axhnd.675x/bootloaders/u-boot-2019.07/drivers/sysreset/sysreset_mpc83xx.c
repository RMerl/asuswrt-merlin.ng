// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2018
 * Mario Six, Guntermann & Drunck GmbH, mario.six@gdsys.cc
 */

#include <common.h>
#include <dm.h>
#include <sysreset.h>
#include <wait_bit.h>

#include "sysreset_mpc83xx.h"

/* Magic 4-byte word to enable reset ('RSTE' in ASCII) */
static const u32 RPR_MAGIC = 0x52535445;
/* Wait at most 2000ms for reset control enable bit */
static const uint RESET_WAIT_TIMEOUT = 2000;

/**
 * __do_reset() - Execute the system reset
 *
 * Return: The functions resets the system, and never returns.
 */
static int __do_reset(void)
{
	ulong msr;
	int res;

	immap_t *immap = (immap_t *)CONFIG_SYS_IMMR;

	puts("Resetting the board.\n");

	/* Interrupts and MMU off */
	msr = mfmsr();
	msr &= ~(MSR_EE | MSR_IR | MSR_DR);
	mtmsr(msr);

	/* Enable Reset Control Reg */
	out_be32(&immap->reset.rpr, RPR_MAGIC);
	sync();
	isync();

	/* Confirm Reset Control Reg is enabled */
	res = wait_for_bit_be32(&immap->reset.rcer, RCER_CRE, true,
				RESET_WAIT_TIMEOUT, false);
	if (res) {
		debug("%s: Timed out waiting for reset control to be set\n",
		      __func__);
		return res;
	}

	udelay(200);

	/* Perform reset, only one bit */
	out_be32(&immap->reset.rcr, RCR_SWHR);

	/* Never executes */
	return 0;
}

static int mpc83xx_sysreset_request(struct udevice *dev, enum sysreset_t type)
{
	switch (type) {
	case SYSRESET_WARM:
	case SYSRESET_COLD:
		return __do_reset();
	default:
		return -EPROTONOSUPPORT;
	}

	return -EINPROGRESS;
}

/**
 * print_83xx_arb_event() - Print arbiter events to buffer
 * @force: Print arbiter events, even if none are indicated by the system
 * @buf:   The buffer to receive the printed arbiter event information
 * @size:  The size of the buffer to receive the printed arbiter event
 *	   information in bytes
 *
 * Return: Number of bytes printed to buffer, -ve on error
 */
static int print_83xx_arb_event(bool force, char *buf, int size)
{
	int etype = (gd->arch.arbiter_event_attributes & AEATR_EVENT)
		    >> AEATR_EVENT_SHIFT;
	int mstr_id = (gd->arch.arbiter_event_attributes & AEATR_MSTR_ID)
		      >> AEATR_MSTR_ID_SHIFT;
	int tbst = (gd->arch.arbiter_event_attributes & AEATR_TBST)
		   >> AEATR_TBST_SHIFT;
	int tsize = (gd->arch.arbiter_event_attributes & AEATR_TSIZE)
		    >> AEATR_TSIZE_SHIFT;
	int ttype = (gd->arch.arbiter_event_attributes & AEATR_TTYPE)
		    >> AEATR_TTYPE_SHIFT;
	int tsize_val = (tbst << 3) | tsize;
	int tsize_bytes = tbst ? (tsize ? tsize : 8) : 16 + 8 * tsize;
	int res = 0;

	/*
	 * If we don't force output, and there is no event (event address ==
	 * 0), then don't print anything
	 */
	if (!force && !gd->arch.arbiter_event_address)
		return 0;

	if (CONFIG_IS_ENABLED(CONFIG_DISPLAY_AER_FULL)) {
		res = snprintf(buf, size,
			       "Arbiter Event Status:\n"
			       "    %s: 0x%08lX\n"
			       "    %s:    0x%1x  = %s\n"
			       "    %s:     0x%02x = %s\n"
			       "    %s: 0x%1x  = %d bytes\n"
			       "    %s: 0x%02x = %s\n",
			       "Event Address", gd->arch.arbiter_event_address,
			       "Event Type", etype, event[etype],
			       "Master ID", mstr_id, master[mstr_id],
			       "Transfer Size", tsize_val, tsize_bytes,
			       "Transfer Type", ttype, transfer[ttype]);
	} else if (CONFIG_IS_ENABLED(CONFIG_DISPLAY_AER_BRIEF)) {
		res = snprintf(buf, size,
			       "Arbiter Event Status: AEATR=0x%08lX, AEADR=0x%08lX\n",
			       gd->arch.arbiter_event_attributes,
			       gd->arch.arbiter_event_address);
	}

	return res;
}

static int mpc83xx_sysreset_get_status(struct udevice *dev, char *buf, int size)
{
	/* Ad-hoc data structure to map RSR bit values to their descriptions */
	static const struct {
		/* Bit mask for the bit in question */
		ulong mask;
		/* Description of the bitmask in question */
		char *desc;
	} bits[] = {
		{
		RSR_SWSR, "Software Soft"}, {
		RSR_SWHR, "Software Hard"}, {
		RSR_JSRS, "JTAG Soft"}, {
		RSR_CSHR, "Check Stop"}, {
		RSR_SWRS, "Software Watchdog"}, {
		RSR_BMRS, "Bus Monitor"}, {
		RSR_SRS,  "External/Internal Soft"}, {
		RSR_HRS,  "External/Internal Hard"}
	};
	int res;
	ulong rsr = gd->arch.reset_status;
	int i;
	char *sep;

	res = snprintf(buf, size, "Reset Status:");
	if (res < 0) {
		debug("%s: Could not write reset status message (err = %d)\n",
		      dev->name, res);
		return -EIO;
	}

	buf += res;
	size -= res;

	sep = " ";
	for (i = 0; i < ARRAY_SIZE(bits); i++)
		/* Print description of set bits */
		if (rsr & bits[i].mask) {
			res = snprintf(buf, size, "%s%s%s", sep, bits[i].desc,
				       (i == ARRAY_SIZE(bits) - 1) ? "\n" : "");
			if (res < 0) {
				debug("%s: Could not write reset status message (err = %d)\n",
				      dev->name, res);
				return -EIO;
			}
			buf += res;
			size -= res;
			sep = ", ";
		}

	/*
	 * TODO(mario.six@gdsys.cc): Move this into a dedicated
	 *			     arbiter driver
	 */
	if (CONFIG_IS_ENABLED(CONFIG_DISPLAY_AER_FULL) ||
	    CONFIG_IS_ENABLED(CONFIG_DISPLAY_AER_BRIEF)) {
		/*
		 * If there was a bus monitor reset event, we force the arbiter
		 * event to be printed
		 */
		res = print_83xx_arb_event(rsr & RSR_BMRS, buf, size);
		if (res < 0) {
			debug("%s: Could not write arbiter event message (err = %d)\n",
			      dev->name, res);
			return -EIO;
		}
		buf += res;
		size -= res;
	}
	snprintf(buf, size, "\n");

	return 0;
}

static struct sysreset_ops mpc83xx_sysreset = {
	.request	= mpc83xx_sysreset_request,
	.get_status	= mpc83xx_sysreset_get_status,
};

U_BOOT_DRIVER(sysreset_mpc83xx) = {
	.name	= "mpc83xx_sysreset",
	.id	= UCLASS_SYSRESET,
	.ops	= &mpc83xx_sysreset,
};
