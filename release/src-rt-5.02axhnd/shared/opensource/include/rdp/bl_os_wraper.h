/*
* <:copyright-BRCM:2012:DUAL/GPL:standard
* 
*    Copyright (c) 2012 Broadcom 
*    All Rights Reserved
* 
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License, version 2, as published by
* the Free Software Foundation (the "GPL").
* 
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
* 
* 
* A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
* writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
* Boston, MA 02111-1307, USA.
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

 #include <linux/kernel.h>
 #include <linux/module.h>
 #include <linux/interrupt.h>
 #include <linux/delay.h>
 #include <linux/slab.h>
 #include <linux/slab_def.h>
 #include <linux/version.h>
 #include <linux/types.h>
 #include <linux/netdevice.h>
 #include <linux/skbuff.h>
 #include <linux/platform_device.h>
 #include <linux/string.h>
 #include <linux/irq.h>
 #include <linux/uaccess.h>

#elif defined _CFE_ROM_
  #include "lib_types.h"

#elif defined _CFE_
 #define EXPORT_SYMBOL(m)   /*  */
 #include "linux/types.h"
 #include "lib_types.h"
 #include "lib_malloc.h"
 #include "lib_printf.h"
 #include "lib_string.h"
 #include "cfe_timer.h"
 #include "addrspace.h"
 #include "lib_swab.h"

 #define memset(d,s,c)	lib_memset(d,s,c)
 #define malloc(c)	KMALLOC(c,0)
 //should be fixed in BDMF 
 #define bdmf_sysb	int

 #ifndef printk
  #define printk	printf
 #endif
 
 #define udelay(usec)	cfe_usleep(usec)
 #define mdelay(n)	({unsigned long _msec=(n);	\
			while(_msec--) udelay(1000);})

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

