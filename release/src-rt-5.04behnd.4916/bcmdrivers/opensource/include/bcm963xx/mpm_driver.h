/*
   Copyright (c) 2020 Broadcom Corporation
   All Rights Reserved

<:label-BRCM:2020:DUAL/GPL:standard

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
 * File Name  : mpm_driver.h
 *
 * Description: This file contains the specification of some common definitions
 *      and interfaces to other modules. This file may be included by both
 *      Kernel and userapp (C only).
 *
 *******************************************************************************
 */

#ifndef __MPM_DRIVER_H_INCLUDED__
#define __MPM_DRIVER_H_INCLUDED__

#define MPM_VERSION              "0.1"
#define MPM_VER_STR              "v" MPM_VERSION
#define MPM_MODNAME              "Broadcom HW Memory Pool Manager (MPM)"

/* MPM Character Device */
#define MPM_DRV_MAJOR            342
#define MPM_DRV_NAME             "mpm"
#define MPM_DRV_DEVICE_NAME      "/dev/" MPM_DRV_NAME

/* MPM Control Utility Executable */
#define MPM_CTL_PATH             "/bin/mpmctl"

/*
 *------------------------------------------------------------------------------
 * Common defines for MPM layers.
 *------------------------------------------------------------------------------
 */
#undef MPM_DECL
#define MPM_DECL(x)                 x,  /* for enum declaration in H file */

/*
 *------------------------------------------------------------------------------
 * Mpm character device driver IOCTL enums
 * A character device and the associated userspace utility for debug.
 *------------------------------------------------------------------------------
 */
typedef enum {
/* IOCTL cmd values 1 and 2 are mapped to FIBMAP and FIGETBSZ on ARM
   processor. Hence start all IOCTL values from 100 to prevent conflicts */
    MPM_IOC_DUMMY=99,
    MPM_DECL(MPM_IOC_STATUS)
    MPM_DECL(MPM_IOC_STATS)
    MPM_DECL(MPM_IOC_DEBUG)
    MPM_DECL(MPM_IOC_DUMP)
    MPM_DECL(MPM_IOC_UT)
    MPM_DECL(MPM_IOC_MAX)
} mpm_ioctl_cmd_t;

typedef enum {
    MPM_UT_TEST_MPM,
    MPM_UT_TEST_MPM_VS_BPM,
    MPM_UT_TEST_MAX
} mpm_test_t;

#endif  /* __MPM_DRIVER_H_INCLUDED__ */
