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

/* uint32_t types */
#include "linux/types.h"

/* udelay */
#include <asm/delay.h>

/* NULL */
#if !defined(NULL)
#define NULL 0
#endif

/* memset */
#include "linux/string.h"

/* printk */
#include "linux/printk.h"

/* spinlock */
#include <linux/spinlock.h>

/* EXPORT_SYMBOL */
#include "linux/export.h"

#endif
