/*
   Copyright (c) 2006-2017 Broadcom Corporation
   All Rights Reserved

<:label-BRCM:2017:DUAL/GPL:standard

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
#define ARCHER_DRV_MAJOR             339
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
    ARCHER_DECL(ARCHER_IOC_BIND)
    ARCHER_DECL(ARCHER_IOC_UNBIND)
    ARCHER_DECL(ARCHER_IOC_DEBUG)
    ARCHER_DECL(ARCHER_IOC_FLOWS)
    ARCHER_DECL(ARCHER_IOC_UCAST_L3)
    ARCHER_DECL(ARCHER_IOC_UCAST_L2)
    ARCHER_DECL(ARCHER_IOC_MCAST)
    ARCHER_DECL(ARCHER_IOC_HOST)
    ARCHER_DECL(ARCHER_IOC_MODE)
    ARCHER_DECL(ARCHER_IOC_STATS)
    ARCHER_DECL(ARCHER_IOC_SYSPORT)
    ARCHER_DECL(ARCHER_IOC_MPDCFG)
    ARCHER_DECL(ARCHER_IOC_WOL)
    ARCHER_DECL(ARCHER_IOC_DPI)
    ARCHER_DECL(ARCHER_IOC_XTMDROPALG_SET)
    ARCHER_DECL(ARCHER_IOC_XTMDROPALG_GET)
    ARCHER_DECL(ARCHER_IOC_MAX)
} archer_ioctl_cmd_t;

typedef enum {
    ARCHER_MODE_L3,
    ARCHER_MODE_L2_L3,
    ARCHER_MODE_MAX
} archer_mode_t;

typedef enum {
    ARCHER_MPD_INTF,
    ARCHER_MPD_ADDR_SPEC,
    ARCHER_MPD_MODE_MAX
} archer_mpdcfg_mode_t;

typedef struct {
    archer_mpdcfg_mode_t   mode;
    char            intf_name[16];
    unsigned char   mac_addr[6];
} archer_mpd_cfg_t;

typedef enum {
    ARCHER_DPI_CMD_ENABLE,
    ARCHER_DPI_CMD_DISABLE,
    ARCHER_DPI_CMD_STATS,
    ARCHER_DPI_CMD_MAX
} archer_dpi_cmd_t;

typedef enum {

    ARCHER_DROPALG_DT = 0,
    ARCHER_DROPALG_RED,
    ARCHER_DROPALG_WRED,

} archer_dropalg_alg_t;

typedef struct {

    uint32_t dropProb;
    uint32_t minThres;
    uint32_t maxThres;

} archer_dropalg_thres_t;


typedef struct {

    uint32_t tx_idx;
    archer_dropalg_alg_t   alg;
    archer_dropalg_thres_t thres_lo;
    archer_dropalg_thres_t thres_hi;

    uint32_t priorityMask_0;
    uint32_t priorityMask_1;

} archer_dropalg_config_t;

typedef enum {
    ARCHER_SYSPORT_CMD_REG_DUMP,
    ARCHER_SYSPORT_CMD_PORT_DUMP,
    ARCHER_SYSPORT_CMD_MAX
} archer_sysport_cmd_t;

#endif  /* __ARCHER_H_INCLUDED__ */
