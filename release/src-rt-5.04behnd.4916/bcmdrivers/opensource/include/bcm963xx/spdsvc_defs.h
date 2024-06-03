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

#include "bcm_async_queue.h"
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
 * Common defines for Speed Service layers
 *------------------------------------------------------------------------------
 */

#define SPDSVC_RET_SUCCESS         0
#define SPDSVC_RET_ERROR          -1
#define SPDSVC_RET_BUSY           -2
#define SPDSVC_RET_NOT_SUPPORTED  -3

typedef enum {
    SPDSVC_MODE_STANDARD,
    SPDSVC_MODE_TR471,
    SPDSVC_MODE_MAX
} spdsvc_mode_t;

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

/* This file is shared between userspace/OBUDPST and Kernel driver
 * loadHdr structure is defined in udpst_protocol.h in userspace application
 * BRCM_UDPST_OFFLOAD is only defined when compiling application */
#if !defined(BRCM_UDPST_OFFLOAD)
#if defined(CONFIG_BCM_TR471_MFLOW)
struct loadHdr {
#define LOAD_ID 0xBEEF
        uint16_t loadId; // Load ID
#define TEST_ACT_TEST  0
#define TEST_ACT_STOP1 1
#define TEST_ACT_STOP2 2
        uint8_t testAction;  // Test action
        uint8_t rxStopped;   // Receive traffic stopped indicator (BOOL)
        uint32_t lpduSeqNo;  // Load PDU sequence number
        uint16_t udpPayload; // UDP payload (bytes)
        uint16_t spduSeqErr; // Status PDU sequence error count
        //
        uint32_t spduTime_sec;  // Send time in last received status PDU
        uint32_t spduTime_nsec; // Send time in last received status PDU
        uint32_t lpduTime_sec;  // Send time of this load PDU
        uint32_t lpduTime_nsec; // Send time of this load PDU
        //
        uint16_t rttRespDelay; // Response delay for RTT calculation (ms)
        uint16_t reserved1;    // (Alignment)
};
#else
struct loadHdr {
#define LOAD_ID 0xBEEF
        uint16_t loadId; // Load ID
#define TEST_ACT_TEST  0
#define TEST_ACT_STOP1 1
#define TEST_ACT_STOP2 2
        uint8_t testAction;  // Test action
        uint8_t rxStopped;   // Receive traffic stopped indicator (BOOL)
        uint32_t lpduSeqNo;  // Load PDU sequence number
        uint16_t udpPayload; // UDP payload (bytes)
        uint16_t spduSeqErr; // Status PDU sequence error count
        //
        uint32_t spduTime_sec;  // Send time in last received status PDU
        uint32_t spduTime_nsec; // Send time in last received status PDU
        uint32_t lpduTime_sec;  // Send time of this load PDU
        uint32_t lpduTime_nsec; // Send time of this load PDU
};
#endif /* OBUDPST_MFLOW */
#endif

#define SPDSVC_TR471_RX_QUEUE_SIZE  256 // Must be power of 2

typedef struct loadHdr spdsvc_tr471_rx_queue_entry_t;

typedef struct {
    spdsvc_tr471_rx_queue_entry_t entry[SPDSVC_TR471_RX_QUEUE_SIZE];
    bcm_async_queue_t async_queue;
} spdsvc_tr471_rx_queue_t;

typedef int (* tr471_rx_queue_write_t)(spdsvc_tr471_rx_queue_entry_t *entry_p);

#define SPDSVC_TR471_RX_QUEUE_PAGE_SIZE  4096 // Kernel page size, in bytes

static inline int spdsvc_tr471_rx_queue_mem_size(void)
{
    int size = sizeof(spdsvc_tr471_rx_queue_t);
    int pages = ((size / SPDSVC_TR471_RX_QUEUE_PAGE_SIZE) +
                 ((size % SPDSVC_TR471_RX_QUEUE_PAGE_SIZE) ? 1 : 0));

    return (pages * SPDSVC_TR471_RX_QUEUE_PAGE_SIZE);
}

typedef struct {
    int totalburst;
    int burstsize;
    unsigned int payload;
    unsigned int addon;
    int randPayload;
    int firstBurst;
    int pid;
    int event_fd;
    int burst_cmpl_event_fd;
    struct loadHdr lHdr;
} spdsvc_config_tr471_t;

typedef struct {
    spdsvc_mode_t mode;
    spdsvc_direction_t dir;            /**< Test direction */
    spdsvc_socket_t socket;
    spdsvc_config_tr471_t tr471;
} spdsvc_config_t;

typedef struct {
    uint8_t running;      /**< 0: Test done; 1: Test in progress */
    uint32_t rx_packets;
    uint32_t rx_bytes;
    uint32_t rx_time_usec;
    uint32_t tx_packets;
    uint32_t tx_discards;
    int tx_setup_done;
    int tx_is_supported;
} spdsvc_result_t;

typedef enum {
    SPDSVC_HEADER_TYPE_ETH = 0,     /* LAN: ETH, WAN: EoA, MER, PPPoE */
    SPDSVC_HEADER_TYPE_ETH_BCMTAG,  /* Ethernet + Broadcom Tag */
    SPDSVC_HEADER_TYPE_PPP,         /*           WAN: PPPoA */
    SPDSVC_HEADER_TYPE_IP,          /*           WAN: IPoA */
    SPDSVC_HEADER_TYPE_MAX
} spdsvc_header_type_t;

typedef enum {
    SPDSVC_EGRESS_TYPE_ENET,
    SPDSVC_EGRESS_TYPE_WLAN_DHD,
    SPDSVC_EGRESS_TYPE_WLAN_NIC,
    SPDSVC_EGRESS_TYPE_DHD_OFFLOAD,
    SPDSVC_EGRESS_TYPE_DSL,
    SPDSVC_EGRESS_TYPE_PON,
    SPDSVC_EGRESS_TYPE_MAX
} spdsvc_egress_type_t; 

#define SPDSVC_HOOK_TRANSMIT_SPDT_NO_AUTO_TRIGGER  (1 << 0) /* Speed Service is not triggered by the driver,
                                                               need to be triggered manually */

typedef int (* spdsvc_transmit_helper_t)(void *pNBuff, uint32_t *tag_p);

typedef struct {
    void *pNBuff;
    void *dev;
    spdsvc_header_type_t header_type;
    uint32_t phy_overhead;
    uint32_t so_mark;
    uint32_t flags;
    spdsvc_transmit_helper_t transmit_helper;
    spdsvc_egress_type_t egress_type;
    uint32_t tag;
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
