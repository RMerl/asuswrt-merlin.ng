/*
   Copyright (c) 2006-2017 Broadcom Corporation
   All Rights Reserved

<:label-BRCM:2017:DUAL/GPL:standard

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
 *******************************************************************************
 * File Name  : archer.h
 *
 * Description: This file contains the specification of some common definitions
 *      and interfaces to other modules. This file may be included by both
 *      Kernel and userapp (C only).
 *
 *******************************************************************************
 */

#ifndef __ARCHER_H_INCLUDED__
#define __ARCHER_H_INCLUDED__

#define ARCHER_VERSION              "0.1"
#define ARCHER_VER_STR              "v" ARCHER_VERSION
#define ARCHER_MODNAME              "Broadcom Archer Network Processor"

/* ARCHER Character Device */
#define ARCHER_DRV_MAJOR             3039
#define ARCHER_DRV_NAME              "archer"
#define ARCHER_DRV_DEVICE_NAME       "/dev/" ARCHER_DRV_NAME

/* ARCHER Control Utility Executable */
#define ARCHER_CTL_PATH             "/bin/archerctl"

#define ARCHER_DONT_CARE        ~0
#define ARCHER_IS_DONT_CARE(_x) ( ((_x) == (typeof(_x))(ARCHER_DONT_CARE)) )

/*
 *------------------------------------------------------------------------------
 * Common defines for ARCHER layers.
 *------------------------------------------------------------------------------
 */
#undef ARCHER_DECL
#define ARCHER_DECL(x)                 x,  /* for enum declaration in H file */

/*
 *------------------------------------------------------------------------------
 * Archer character device driver IOCTL enums
 * A character device and the associated userspace utility for debug.
 *------------------------------------------------------------------------------
 */
typedef enum {
/* IOCTL cmd values 1 and 2 are mapped to FIBMAP and FIGETBSZ on ARM
   processor. Hence start all IOCTL values from 100 to prevent conflicts */
    ARCHER_IOC_DUMMY=99,
    ARCHER_DECL(ARCHER_IOC_STATUS)
    ARCHER_DECL(ARCHER_IOC_ENABLE)
    ARCHER_DECL(ARCHER_IOC_DISABLE)
    ARCHER_DECL(ARCHER_IOC_DEBUG)
    ARCHER_DECL(ARCHER_IOC_FLOWS)
    ARCHER_DECL(ARCHER_IOC_HOST)
    ARCHER_DECL(ARCHER_IOC_MODE)
    ARCHER_DECL(ARCHER_IOC_STATS)
    ARCHER_DECL(ARCHER_IOC_MAX)
} archer_ioctl_cmd_t;

typedef enum {
    ARCHER_MODE_L3,
    ARCHER_MODE_L2_L3,
    ARCHER_MODE_MAX
} archer_mode_t;

#endif  /* __ARCHER_H_INCLUDED__ */
