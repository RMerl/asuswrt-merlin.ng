// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2012 Broadcom
 */
/*
*  
*/

/************************************************************************/
/*									*/
/* OS abstraction							*/
/*									*/
/************************************************************************/

#ifndef _BL_OS_WRAPER_H_
#define _BL_OS_WRAPER_H_


#if defined __UBOOT__
 #include <common.h>
 #define bdmf_sysb	int
 #define xprintf	printf
#elif defined __KERNEL__

 #include <linux/kernel.h>
 #include <linux/module.h>
 #include <linux/interrupt.h>
 #include <linux/delay.h>
 #include <linux/slab.h>
 #include <linux/version.h>
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 19, 0)
 #include <linux/mm_types.h>
#endif
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

