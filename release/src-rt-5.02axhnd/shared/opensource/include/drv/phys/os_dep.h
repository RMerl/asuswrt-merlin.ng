/*
   Copyright (c) 2015 Broadcom Corporation
   All Rights Reserved

    <:label-BRCM:2015:DUAL/GPL:standard
    
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

/*
 *  Created on: Dec 2015
 *      Author: yuval.raviv@broadcom.com
 */

#ifndef __OS_DEP_H__
#define __OS_DEP_H__

/* udelay */
#ifdef _CFE_
extern void cfe_usleep(int usec);
#ifndef udelay
#define udelay(x) cfe_usleep(x)
#endif
#else
#include <asm/delay.h>
#endif

/* NULL */
#ifdef _CFE_
#define NULL 0
#endif

/* uint32_t types */
#ifdef _CFE_
#include "lib_types.h"
#else
#include "linux/types.h"
#endif

/* memset */
#ifdef _CFE_
#include "lib_string.h"
#endif

/* printk */
#ifdef _CFE_
#include "lib_printf.h"
#define printk printf
#else
#include "linux/printk.h"
#endif

/* EXPORT_SYMBOL */
#ifdef _CFE_
#ifndef EXPORT_SYMBOL
#define EXPORT_SYMBOL(x)
#endif
#else
#include "linux/export.h"
#endif

#endif
