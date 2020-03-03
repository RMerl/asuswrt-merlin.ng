#if defined(CONFIG_BCM_KF_TSTAMP)
/*
<:copyright-BRCM:2011:GPL/GPL:standard

   Copyright (c) 2011 Broadcom 
   All Rights Reserved

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License, version 2, as published by
the Free Software Foundation (the "GPL").

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.


A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.

:>
*/
#include <linux/module.h>
#include <linux/init.h>
#include <linux/bcm_tstamp.h>
#include <asm/time.h>

static u32 _2us_divisor;
static u32 _2ns_shift;
static u32 _2ns_multiplier;

u32 bcm_tstamp_read(void)
{
	return read_c0_count();
}
EXPORT_SYMBOL(bcm_tstamp_read);


u32 bcm_tstamp_delta(u32 start, u32 end)
{
	// start and end could have been read from different CPU's.
	// Typically, I have seen the counters on the CPU's to be within
	// 20 cycles of each other.  Allow a bit more for a margin of error
	if (start <= 100 && ((end > 0xffffffc0) ||
	                     (start >= end && end <= 100)))
		return 1;
	else if (end > start)
		return end-start;  // simplest case
	else if (start > 100 && (start-end < 100))
		return 1;
	else
		return (0xffffffff - start + end);  // simple rollover
}
EXPORT_SYMBOL(bcm_tstamp_delta);


u32 bcm_tstamp_elapsed(u32 start)
{
	u32 end = read_c0_count();
	return bcm_tstamp_delta(start, end);
}
EXPORT_SYMBOL(bcm_tstamp_elapsed);


u32 bcm_tstamp2us(u32 i)
{
	return (i/_2us_divisor);
}
EXPORT_SYMBOL(bcm_tstamp2us);


u64 bcm_tstamp2ns(u32 i)
{
	u64 ns = (u64) i;
	ns = (ns * _2ns_multiplier) >> _2ns_shift;
	return ns;
}
EXPORT_SYMBOL(bcm_tstamp2ns);


int __init init_bcm_tstamp(void)
{
	if (mips_hpt_frequency == 0)
		mips_hpt_frequency = 160000000;

	_2us_divisor = mips_hpt_frequency / 1000000;

	if (mips_hpt_frequency == 200000000) { //400MHz
		_2ns_multiplier = 5;  //5ns
		_2ns_shift = 0;
	} else if (mips_hpt_frequency == 166500000) { //333MHz
		_2ns_multiplier = 6;  //6ns
		_2ns_shift = 0;
	} else if (mips_hpt_frequency == 160000000) { //320MHz
		_2ns_multiplier = 25;  //6.25ns
		_2ns_shift = 2;
	} else if (mips_hpt_frequency == 15000000) { //300MHz
		_2ns_multiplier = 13;  // approximate to 6.5? actual is 6.667ns
		_2ns_shift = 1;
	} else {
		printk("init_bcm_tstamp: unhandled mips_hpt_freq=%d, "
		       "adjust constants in bcm_tstamp.c\n", mips_hpt_frequency);
	}

	printk(KERN_INFO "bcm_tstamp initialized, (hpt_freq=%d 2us_div=%u "
	                 "2ns_mult=%u 2ns_shift=%u)\n", mips_hpt_frequency,
	                 _2us_divisor, _2ns_multiplier, _2ns_shift);

	return 0;
}
__initcall(init_bcm_tstamp);

#endif /* defined(CONFIG_BCM_KF_TSTAMP) */

