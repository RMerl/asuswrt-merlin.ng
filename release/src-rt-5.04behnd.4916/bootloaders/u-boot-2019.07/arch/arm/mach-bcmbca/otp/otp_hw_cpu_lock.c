/* SPDX-License-Identifier: GPL-2.0+
 *
 *  Copyright 2020 Broadcom Ltd.
 */


/**********************************************************************
 *	
 * OTP HW CPU lock semaphore  
 *	
 *********************************************************************	
 */
#include <common.h>
#include <linux/types.h>
#include <linux/io.h>
#include "linux/printk.h"
#include <asm/arch/misc.h>
#include <asm/arch/otp.h>
#include "otp_hw.h" 
#include "otp_map_cmn.h" 


otp_hw_cmn_err_t otp_hw_cpu_lock(otp_hw_cmn_t* dev)
{
	/*  HW Lock the OTP controller
	*   Until lock is obtained the HW will NOT PERMIT the CPU to access 
	*   OTP control registers
 	**/
	int to = OTP_CPU_LOCK_TMO_CNT;
	writel(0x1, dev->mm + OTP_CPU_LOCK_OFFSET);
	while( to && 
		!((readl(dev->mm + OTP_CPU_LOCK_OFFSET)>>OTP_CPU_LOCK_SHIFT) & OTP_CPU_LOCK_MASK))  {
		udelay(1);
		to--;
	}
	if ( to < 1) {
		printf("%s: Error! Timed out waiting for OTP_CPU_LOCK!\n", __FUNCTION__);
		return OTP_HW_CMN_ERR_TMO;
	}
	return OTP_HW_CMN_OK;
}

void otp_hw_cpu_unlock(otp_hw_cmn_t* dev)
{
	/* Release hardware spinlock for OTP */
	writel(0x0, dev->mm + OTP_CPU_LOCK_OFFSET);
}

