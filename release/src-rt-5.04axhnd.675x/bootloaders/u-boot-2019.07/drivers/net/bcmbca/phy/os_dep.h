// SPDX-License-Identifier: GPL-2.0+
/*
   Copyright (c) 2015 Broadcom Corporation
   All Rights Reserved

    
*/

/*
 *  Created on: Dec 2015
 *      Author: yuval.raviv@broadcom.com
 */

#ifndef __OS_DEP_H__
#define __OS_DEP_H__

/* uint32_t types */
#ifdef __UBOOT__
#include "linux/types.h"
#else
#include "linux/types.h"
#endif

/* udelay */
#ifdef __UBOOT__
extern uint32_t jiffies;
#else
#include <asm/delay.h>
#endif

/* NULL */
#if !defined(NULL)
#define NULL 0
#endif

/* memset */
#ifdef __UBOOT__
#include <string.h>
#else
#include "linux/string.h"
#endif

/* printk */
#ifdef __UBOOT__
#include <stdio.h>
#include "linux/printk.h"
#else
#include "linux/printk.h"
#endif

/* spinlock */
#ifdef __UBOOT__
#define spin_lock_bh(x) *x = 1
#define spin_unlock_bh(x) *x = 0
#define DEFINE_SPINLOCK(x) int x
#else
#include <linux/spinlock.h>
#endif

/* EXPORT_SYMBOL */
#ifdef __UBOOT__
#include "linux/compat.h"
#else
#include "linux/export.h"
#endif

#endif
