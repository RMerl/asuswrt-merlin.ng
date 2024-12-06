/*
   Copyright (c) 2020 Broadcom Corporation
   All Rights Reserved

<:label-BRCM:2020:DUAL/GPL:standard

Unless you and Broadcom execute a separate written software license
agreement governing use of this software, this software is licensed
to you under the terms of the GNU General Public License version 2
(the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
with the following added to such license:

   As a special exception, the copyright holders of this software give
   you permission to link this software with independent modules, and
   to copy and distribute the resulting executable under terms of your
   choice, provided that you also meet, for each linked independent
   module, the terms and conditions of the license of that module.
   An independent module is a module which is not derived from this
   software.  The special exception does not apply to any modifications
   of the software.

Not withstanding the above, under no circumstances may you combine
this software in any way with any other Broadcom software provided
under a license other than the GPL, without Broadcom's express prior
written consent.

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
