/*
    Copyright 2007-2015 Broadcom Corporation

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

/***************************************************************************
 * File Name  : bcmspudrv.h
 *
 * Description: This file contains the definitions and structures for the
 *              Linux IOCTL interface that used between the user mode SPU
 *              API library and the kernel SPU API driver.
 *
 * Updates    : 11/26/2007  Pavan Kumar.  Created.
 ***************************************************************************/

#if !defined(_BCMSPUDRV_H_)
#define _BCMSPUDRV_H_

/* Includes. */
#include <bcmspucfg.h>

#if defined(__cplusplus)
extern "C" {
#endif

#define IPSECSPUDRV_IOC_MAGIC   233

#define SPUDDIOCTL_INITIALIZE   _IOWR(IPSECSPUDRV_IOC_MAGIC, 0, SPUDDDRV_INITIALIZE)
#define SPUDDIOCTL_UNINITIALIZE _IOR(IPSECSPUDRV_IOC_MAGIC, 1, SPUDDDRV_STATUS_ONLY)
#define SPUDDIOCTL_SPU_SHOW     _IOR(IPSECSPUDRV_IOC_MAGIC, 2, SPUDDDRV_SPU_SHOW)
#define SPUDDIOCTL_TEST         _IOR(IPSECSPUDRV_IOC_MAGIC, 3, SPUDDDRV_TEST)

#define MAX_SPUDDDRV_IOCTL_COMMANDS 4

/* Enumerations */

/* Globals */

/* Typedefs. */
typedef struct
{
    SPU_STATUS      bvStatus;
} SPUDDDRV_STATUS_ONLY, *PSPUDDDRV_STATUS_ONLY;

typedef struct
{
    SPU_STATUS      bvStatus;
} SPUDDDRV_INITIALIZE, *PSPUDDDRV_INITIALIZE;

typedef struct
{
    SPU_STAT_PARMS  stats;
    SPU_STATUS      bvStatus;
} SPUDDDRV_SPU_SHOW, *PSPUDDDRV_SPU_SHOW;

typedef struct
{
    SPU_TEST_PARMS  testParams;
    SPU_STATUS      bvStatus;
} SPUDDDRV_TEST, *PSPUDDDRV_TEST;

#if defined(__cplusplus)
}
#endif
#endif // _BCMSPUDRV_H_
