/* SPDX-License-Identifier: GPL-2.0+
 *
 *  Copyright 2020 Broadcom Ltd.
 */

/**********************************************************************
 *	
 * OTP hw ecc function  
 *
 *	
 ********************************************************************	
 */
#include <common.h>
#include <linux/types.h>
#include "linux/io.h"
#include "linux/printk.h"
#include <asm/arch/misc.h>
#include <asm/arch/otp.h>
#include "otp_hw.h"
#include "otp_map_cmn.h"

//#define OTP_DRY_RUN

otp_hw_cmn_err_t otp_hw_set_ecc(otp_hw_cmn_t* dev, u32 ecc)
{
	//printf("%s FUSE ECC 0x%x \n", __FUNCTION__,ecc);
#ifndef OTP_DRY_RUN
	writel(ecc, dev->mm + OTP_CTRL2_HI_OFFSET);
#endif
	return OTP_HW_CMN_OK;
}

otp_hw_cmn_err_t otp_hw_get_ecc(otp_hw_cmn_t* dev, u32* ecc)
{
	*ecc = readl(dev->mm + OTP_STATUS0_HI_OFFSET);
	return OTP_HW_CMN_OK;
}
