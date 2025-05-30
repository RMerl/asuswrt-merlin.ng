/*
* <:copyright-BRCM:2012:DUAL/GPL:standard
* 
*    Copyright (c) 2012 Broadcom 
*    All Rights Reserved
* 
* Unless you and Broadcom execute a separate written software license
* agreement governing use of this software, this software is licensed
* to you under the terms of the GNU General Public License version 2
* (the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
* with the following added to such license:
* 
*    As a special exception, the copyright holders of this software give
*    you permission to link this software with independent modules, and
*    to copy and distribute the resulting executable under terms of your
*    choice, provided that you also meet, for each linked independent
*    module, the terms and conditions of the license of that module.
*    An independent module is a module which is not derived from this
*    software.  The special exception does not apply to any modifications
*    of the software.
* 
* Not withstanding the above, under no circumstances may you combine
* this software in any way with any other Broadcom software provided
* under a license other than the GPL, without Broadcom's express prior
* written consent.
* 
* :> 
*/

/************************************************************************/
/*									*/
/* OS abstraction							*/
/*									*/
/************************************************************************/

#ifndef _BL_OS_WRAPER_H_
#define _BL_OS_WRAPER_H_


#if defined __KERNEL__

 #include <linux/kconfig.h>
 #include <linux/kernel.h>
 #include <linux/module.h>
 #include <linux/interrupt.h>
 #include <linux/delay.h>
 #include <linux/slab.h>
 #include <linux/version.h>
 #include <linux/mm_types.h>
 #include <linux/slab_def.h>
 #include <linux/version.h>
 #include <linux/types.h>
 #include <linux/netdevice.h>
 #include <linux/skbuff.h>
 #include <linux/platform_device.h>
 #include <linux/string.h>
 #include <linux/irq.h>
 #include <linux/uaccess.h>

#else
 #warning "Unspecified OS !"
#endif

#define BL_ZERO_REG(reg) 	do {*(unsigned int*)reg = 0;} while(0)

#define ONE_SEC_IN_NANO		1000000000
#define	BL_LILAC_IRQ_BASE	0x70

#ifdef DEBUG
#define BL_ASSERT(x)	if (!(x)) {\
				printk("Assertion in %s at line %d failed/n", __FILE__, __LINE__); \
				while(1); \
			}
#else
#define BL_ASSERT(x)
#endif

#endif /* #ifndef _BL_OS_WRAPER_H_ */

