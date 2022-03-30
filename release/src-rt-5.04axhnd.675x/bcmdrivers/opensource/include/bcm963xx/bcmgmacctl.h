#ifndef __BCMGMACCTL_H_INCLUDED__
#define __BCMGMACCTL_H_INCLUDED__
/*
<:copyright-BRCM:2012:DUAL/GPL:standard

   Copyright (c) 2012 Broadcom 
   All Rights Reserved

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
