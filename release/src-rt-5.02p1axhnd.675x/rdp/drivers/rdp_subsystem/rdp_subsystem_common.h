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

#ifndef __RDP_SUBSYSTEM_COMMON__
#define __RDP_SUBSYSTEM_COMMON__

#if defined(__KERNEL__) || defined(_CFE_ROM_) || defined(_CFE_)

#if defined(_CFE_)
#include "lib_printf.h"
#include "cfe_timer.h"
#include "bcm_hwdefs.h"

#ifndef printk
  #define printk printf
#endif

#ifndef EXPORT_SYMBOL
#define EXPORT_SYMBOL(m) /* */
#endif

#ifndef mdelay
#define mdelay(n) ({unsigned long _msec=(n); while(_msec--) cfe_usleep(1000);})
#endif

#elif defined(__KERNEL__)
#include "bcm_hwdefs.h"
#endif

#elif defined(RDP_SIM)

#include "bdmf_system.h"

#define mdelay(n) ({unsigned long _msec=(n); while(_msec--) bdmf_usleep(1000);})

#endif

#endif
