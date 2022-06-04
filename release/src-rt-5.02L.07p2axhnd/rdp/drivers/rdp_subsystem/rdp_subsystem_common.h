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
