/*
    Copyright 2007-2015 Broadcom Corporation

    <:label-BRCM:2015:DUAL/GPL:standard
    
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
