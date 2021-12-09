#ifndef __SPDSVC_DEFS_H_INCLUDED__
#define __SPDSVC_DEFS_H_INCLUDED__

/*
  Copyright (c) 2015 Broadcom Corporation
  All Rights Reserved

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

/*
*******************************************************************************
* File Name  : spdsvc_defs.h
*
* Description: This file contains the specification of some common definitions
*      and interfaces to other modules. This file may be included by both
*      Kernel and userapp (C only).
*
*******************************************************************************
*/

#include <pktHdr.h>

/*----- Defines -----*/

#define SPDSVC_VERSION              "0.1"
#if defined (__KERNEL__)
#if LINUX_VERSION_CODE > KERNEL_VERSION(3, 4, 0)
#define SPDSVC_VER_STR              "v" SPDSVC_VERSION
#else
#define SPDSVC_VER_STR              "v" SPDSVC_VERSION " " __DATE__ " " __TIME__
#endif
#else
#define SPDSVC_VER_STR              "v" SPDSVC_VERSION " " __DATE__ " " __TIME__
#endif
#define SPDSVC_MODNAME              "Broadcom Speed Service Driver (spdsvc)"

#define SPDSVC_NAME                 "spdsvc"

#define SPDSVC_ERROR                (-1)
#define SPDSVC_SUCCESS              0

/* BCM TM Character Device */
#define SPDSVC_DRV_MAJOR            328
#define SPDSVC_DRV_NAME             SPDSVC_NAME
#define SPDSVC_DRV_DEVICE_NAME      "/dev/" SPDSVC_NAME

/* BCM TM Control Utility Executable */
#define SPDSVC_CTL_UTILITY_PATH     "/bin/spdsvc"

/* BCM TM Proc FS Directory Path */
#define SPDSVC_PROC_FS_DIR_PATH     SPDSVC_NAME

#if defined( __KERNEL__ )
#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 4, 0)
#include <asm/system.h>
#else
#include <asm/cmpxchg.h>
#endif
#define SPDSVC_KERNEL_LOCK(level)          local_irq_save(level)
#define SPDSVC_KERNEL_UNLOCK(level)        local_irq_restore(level)
#endif

#define SPDSVC_DONT_CARE        ~0
#define SPDSVC_IS_DONT_CARE(_x) ( ((_x) == (typeof(_x))(SPDSVC_DONT_CARE)) )


/*
 *------------------------------------------------------------------------------
 * Common defines for BCM TM layers.
 *------------------------------------------------------------------------------
 */

typedef enum {
    SPDSVC_FAMILY_IPV4,
    SPDSVC_FAMILY_IPV6,
    SPDSVC_FAMILY_MAX,
} spdsvc_ip_family_t;

typedef union {
    uint8_t  u8[4];
    uint16_t u16[2];
    uint32_t u32;
} spdsvc_ipv4_addr_t;

typedef union {
    uint8_t  u8[16];
    uint16_t u16[8];
    uint32_t u32[4];
} spdsvc_ipv6_addr_t;

typedef struct {
    spdsvc_ip_family_t family;
    union {
        spdsvc_ipv4_addr_t ipv4;
        spdsvc_ipv6_addr_t ipv6;
    };
} spdsvc_ip_addr_t;

typedef enum {
    SPDSVC_DIRECTION_DS,
    SPDSVC_DIRECTION_US,
    SPDSVC_DIRECTION_MAX,
} spdsvc_direction_t;

typedef struct {
    spdsvc_ip_addr_t local_ip_addr;    /**< Local Ip address */
    uint16_t  local_port_nbr;          /**< Local port number */
    spdsvc_ip_addr_t remote_ip_addr;   /**< Remote Ip address */
    uint16_t  remote_port_nbr;         /**< Remote port number */
} spdsvc_socket_t;

typedef struct {
    spdsvc_direction_t dir;            /**< Test direction */
    spdsvc_socket_t socket;
} spdsvc_config_t;

typedef struct {
    uint8_t running;      /**< 0: Test done; 1: Test in progress */
    uint32_t rx_packets;
    uint32_t rx_bytes;
    uint32_t rx_time_usec;
    uint32_t tx_packets;
    uint32_t tx_discards;
} spdsvc_result_t;

typedef enum {
    SPDSVC_HEADER_TYPE_ETH = 0,     /* LAN: ETH, WAN: EoA, MER, PPPoE */
    SPDSVC_HEADER_TYPE_ETH_BCMTAG,  /* Ethernet + Broadcom Tag */
    SPDSVC_HEADER_TYPE_PPP,         /*           WAN: PPPoA */
    SPDSVC_HEADER_TYPE_IP,          /*           WAN: IPoA */
    SPDSVC_HEADER_TYPE_MAX
} spdsvc_header_type_t;

typedef struct {
    void *pNBuff;
    void *dev;
    spdsvc_header_type_t header_type;
    uint32_t phy_overhead;
} spdsvcHook_transmit_t;

typedef struct {
    void *pNBuff;
    spdsvc_header_type_t header_type;
    uint32_t phy_overhead;
} spdsvcHook_receive_t;

typedef enum {
    SPDSVC_IOCTL_ENABLE=100,
    SPDSVC_IOCTL_GET_OVERHEAD,
    SPDSVC_IOCTL_GET_RESULT,
    SPDSVC_IOCTL_DISABLE,
    SPDSVC_IOCTL_MAX
} spdsvc_ioctl_t;

typedef union {
    spdsvc_config_t config;
    spdsvc_result_t result;
    uint32_t overhead;
} spdsvc_ioctl_arg_t;

#endif  /* defined(__SPDSVC_DEFS_H_INCLUDED__) */
