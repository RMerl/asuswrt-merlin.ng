// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2012 Broadcom
 */
/*
* 
*/

#ifndef __RDP_SUBSYSTEM_COMMON__
#define __RDP_SUBSYSTEM_COMMON__

#if defined(__KERNEL__) || defined(__UBOOT__)

#include <common.h>
#include <linux/compat.h>
#include <linux/delay.h>

#ifndef printk
  #define printk printf
#endif

#elif defined(RDP_SIM)

#include "bdmf_system.h"

#define mdelay(n) ({unsigned long _msec=(n); while(_msec--) bdmf_usleep(1000);})

#endif

#endif
