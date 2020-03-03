#ifndef __BCMGMACCTL_H_INCLUDED__
#define __BCMGMACCTL_H_INCLUDED__
/*
<:copyright-BRCM:2012:DUAL/GPL:standard

   Copyright (c) 2012 Broadcom 
   All Rights Reserved

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
 * File Name : bcmgmacctl.h
 *
 *******************************************************************************
 */

#define GMAC_NAME                 "gmac"
#define GMAC_DRV_NAME             GMAC_NAME
#define GMAC_DRV_DEVICE_NAME      "/dev/" GMAC_DRV_NAME

#define GMACCTL_ERROR             (-1)
#define GMACCTL_SUCCESS           0

/*
 * Ioctl definitions.
 */
typedef enum {
/* IOCTL cmd values 1 and 2 are mapped to FIBMAP and FIGETBSZ on ARM
   processor. Hence start all IOCTL values from 100 to prevent conflicts */
    GMACCTL_IOCTL_SYS=100,
    GMACCTL_IOCTL_MAX
} gmacctl_ioctl_t;

typedef enum {
    GMACCTL_SUBSYS_STATUS,
    GMACCTL_SUBSYS_MODE,
    GMACCTL_SUBSYS_MIB,
    GMACCTL_SUBSYS_MAX
} gmacctl_subsys_t;

typedef enum {
    GMACCTL_OP_GET,
    GMACCTL_OP_SET,
    GMACCTL_OP_DUMP,
    GMACCTL_OP_MAX
} gmacctl_op_t;

typedef struct {
    gmacctl_subsys_t  subsys;
    gmacctl_op_t      op;
    union {
        int           mode;
        int           mib;
    };
} gmacctl_data_t;


#endif  /* defined(__BCMGMACCTL_H_INCLUDED__) */
