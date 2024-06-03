/*
 * <:copyright-BRCM:2019:DUAL/GPL:standard
 * 
 *    Copyright (c) 2019 Broadcom 
 *    All Rights Reserved
 * 
 * Unless you and Broadcom execute a separate written software license
 * agreement governing use of this software, this software is licensed
 * to you under the terms of the GNU General Public License version 2
 * (the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
 * with the following added to such license:
 * 
 *    As a special exception, the copyright holders of this software give
 *    you permission to link this software with independent modules, and
 *    to copy and distribute the resulting executable under terms of your
 *    choice, provided that you also meet, for each linked independent
 *    module, the terms and conditions of the license of that module.
 *    An independent module is a module which is not derived from this
 *    software.  The special exception does not apply to any modifications
 *    of the software.
 * 
 * Not withstanding the above, under no circumstances may you combine
 * this software in any way with any other Broadcom software provided
 * under a license other than the GPL, without Broadcom's express prior
 * written consent.
 * 
 * :>
 */

/*******************************************************************
 * bdmf_types.h
 *
 * Broadcom Device Management Framework - built-in attribute types
 *
 *******************************************************************/

#ifndef BDMF_DATA_TYPES_H_
#define BDMF_DATA_TYPES_H_

#include "bcm_rdp_arch.h"

#if defined(RDP_ARCH_SIM)
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <limits.h>
    typedef long bdmf_index;
    typedef unsigned long bdmf_ptr;
    /** Managed object type handle */
    typedef struct bdmf_type *bdmf_type_handle;

    /** Managed object */
    typedef struct bdmf_object *bdmf_object_handle;

    /** Inter-object link */
    typedef struct bdmf_link *bdmf_link_handle;

    /** Handle that can represent different things */
    typedef void *bdmf_handle;
#elif defined(RDP_ARCH_BOARD) || defined(RDP_ARCH_BOOT) || defined (RDP_ARCH_QEMU_SIM)
#if defined LINUX_KERNEL || defined(__KERNEL__)
/*kernel module defines and includes */
#include <linux/types.h>
    typedef long bdmf_index;
    typedef unsigned long bdmf_ptr;
    /** Managed object type handle */
    typedef struct bdmf_type *bdmf_type_handle;

    /** Managed object */
    typedef struct bdmf_object *bdmf_object_handle;

    /** Inter-object link */
    typedef struct bdmf_link *bdmf_link_handle;

    /** Handle that can represent different things */
    typedef void *bdmf_handle;
#else
#ifndef _CFE_
/*user space defines and includes*/
#include <stdio.h>
#include <stdint.h>
#endif
#if defined KERNEL_64
    typedef uint64_t bdmf_index;
    typedef uint64_t bdmf_ptr;
    typedef uint64_t bdmf_type_handle;
    typedef uint64_t bdmf_object_handle;
    typedef uint64_t bdmf_link_handle;
    typedef uint64_t bdmf_handle;
#else
    typedef long bdmf_index;
    typedef uint32_t bdmf_ptr;
    typedef uint32_t bdmf_type_handle;
    typedef uint32_t bdmf_object_handle;
    typedef uint32_t bdmf_link_handle;
    typedef uint32_t bdmf_handle;
#endif /* KERNEL_64 */
#endif /* LINUX_KERNEL */
#else /* no RDP arch */
#error "no arch defined"
#endif /* RDP_ARCH_SIM */

/** Signed integer
 */
typedef int64_t bdmf_number;

/** System buffer */
typedef void *bdmf_sysb;

#define BDMF_INDEX_UNASSIGNED   (-1)    /**< "Unassigned" bdmf_index value */
#define BDMF_INDEX_ANY          (-2)    /**< "Any" bdmf_index value */

/** String */
typedef char *bdmf_string;

/** Ethernet address */
typedef struct {
    uint8_t b[6];  /**< Address bytes */
} bdmf_mac_t;

/** Check if MAC address is zero
  * param[in]   ip  IP address
  */
static inline int bdmf_mac_is_zero(const bdmf_mac_t *mac)
{
    return !mac->b[0] && !mac->b[1] && !mac->b[2] && !mac->b[3] && !mac->b[4] && !mac->b[5];
}

/** Enumeration value */
typedef long bdmf_enum;

/** Boolean value */
typedef char bdmf_boolean;

/** IP address family */
typedef enum {
    bdmf_ip_family_ipv4,
    bdmf_ip_family_ipv6
} bdmf_ip_family;

/** IPv4 address */
typedef uint32_t bdmf_ipv4;

/** IPv6 address */
typedef struct
{
    uint8_t data[16];
} bdmf_ipv6_t;

/** IPv4 or IPv6 address */
typedef struct {
    bdmf_ip_family family;      /**< Address family: IPv4 / IPv6 */
    union {
        bdmf_ipv4 ipv4;         /**< IPv4 address */
        bdmf_ipv6_t ipv6;       /**< IPv6 address */
    } addr;
} bdmf_ip_t;

/** Check if IPv6 address is zero
  * param[in]   ip  IPv6 address
  */
static inline int bdmf_ipv6_is_zero(const bdmf_ipv6_t *ipv6)
{
    uint32_t *ipv6_as_int = (uint32_t *)ipv6;
    return !ipv6_as_int[0] && !ipv6_as_int[1] && !ipv6_as_int[2] && !ipv6_as_int[3];
}

/** Check if IP address is zero
  * param[in]   ip  IP address
  */
static inline int bdmf_ip_is_zero(const bdmf_ip_t *ip)
{
    if (ip->family == bdmf_ip_family_ipv4)
        return !ip->addr.ipv4;
    return bdmf_ipv6_is_zero(&ip->addr.ipv6);
}

/** Compare IP addresses
  * param[in]   ip  IP address
  * param[in]   ip2  IP address
  */
static inline int bdmf_ip_cmp(const bdmf_ip_t *ip, const bdmf_ip_t *ip2)
{
    uint32_t *ipv6_as_int, *ipv6_as_int2;

    if (ip->family != ip2->family)
        return -1;
    if (ip->family == bdmf_ip_family_ipv4)
        return ip->addr.ipv4 != ip2->addr.ipv4;

    ipv6_as_int = (uint32_t *)&ip->addr.ipv6;
    ipv6_as_int2 = (uint32_t *)&ip2->addr.ipv6;
    return !(ipv6_as_int[0] == ipv6_as_int2[0] && ipv6_as_int[1] == ipv6_as_int2[1] && 
        ipv6_as_int[2] == ipv6_as_int2[2] && ipv6_as_int[3] == ipv6_as_int2[3]);
}

#if (defined(PHYS_ADDR_64BIT) || defined(CONFIG_PHYS_ADDR_T_64BIT)) && !defined(WL4908)
typedef uint64_t bdmf_phys_addr_t;
#else
typedef uint32_t bdmf_phys_addr_t;
#endif

/** Rate */
typedef uint64_t bdmf_rate_t;

static inline uint64_t subtract_uint36(uint64_t l, uint64_t r)
{
    return (l-r) & ((1ULL << 36) - 1);
}

static inline uint64_t subtract_uint32(uint64_t l, uint64_t r)
{
    return (l-r) & ((1ULL << 32) - 1);
}

#endif /* BDMF_DATA_TYPES_H_ */
