/***********************************************************************
 *
 *  Copyright (c) 2021  Broadcom Corporation
 *  All Rights Reserved
 *
 * <:label-BRCM:2012:DUAL/GPL:standard
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License, version 2, as published by
 * the Free Software Foundation (the "GPL").
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * 
 * A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
 * writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 * 
 * :>
 *
************************************************************************/

#ifndef MACSEC_TYPES_H
#define MACSEC_TYPES_H

/*
 * Define unsigned and signed integers with guaranteed sizes.
 * Adjust if your compiler uses different sizes for short or int.
 */
typedef unsigned char       uint8;        /* 8-bit quantity  */
typedef unsigned short      uint16;        /* 16-bit quantity */
typedef unsigned int        uint32;        /* 32-bit quantity */
typedef unsigned long long  COMPILER_UINT64; /* 64-bit quantity */
typedef signed char         int8;         /* 8-bit quantity  */
typedef signed short        int16;        /* 16-bit quantity */
typedef signed int          int32;        /* 32-bit quantity */
typedef signed long long    COMPILER_INT64;    /* 64-bit quantity */
typedef unsigned int        uint32_t;
typedef unsigned int        buint32_t;
typedef unsigned long long  uint64;

typedef int                 soc_block_t;
typedef int                 soc_port_t;
typedef unsigned int        bcm_port_t;
typedef unsigned int        bcm_gport_t;
typedef unsigned short      bcm_vlan_t;

typedef uint8               bcm_mac_t[6];
typedef uint8               sal_mac_addr_t[6];      /* MAC address */

typedef uint32              bcm_ip_t;
typedef unsigned short      bcm_ethertype_t;

/*
 * Define platform-independent types
 */
#ifndef TRUE
#define TRUE        1
#endif

#ifndef FALSE
#define FALSE       0
#endif

#ifndef NULL
#define NULL        0
#endif

#ifndef DONT_CARE
#define DONT_CARE   0
#endif

#define VOL         volatile

/*
 * Generic networking defines 
 */
typedef struct bcm_ipv4_ipv6_addr_s
{
    bcm_ip_t ipv4_addr;     /* Ip address (IPv4). */
    uint8 ipv6_addr[16];    /* Ip address (IPv6). */
} bcm_ipv4_ipv6_addr_t;

typedef struct bcm_ip_addr_s
{
    int protocol; 
    bcm_ipv4_ipv6_addr_t addr; 
} bcm_ip_addr_t;

typedef int sal_mutex_t;

/* 
 * SOC UBUS mapped register descriptors
 */
typedef struct soc_ubus_field_s
{
    uint64  fld_mask;
    int     fld_shift;
} soc_ubus_field_s;
typedef int soc_ubus_field_t;

typedef struct soc_ubus_reg_s
{
    uint32 offset;
    int reg_size;
    int num_flds;
    struct soc_ubus_field_s *reg_flds; 
} soc_ubus_reg_s;

/* 
 * BCM API error codes.
 * 
 * Note: An error code may be converted to a string by passing the code
 * to bcm_errmsg().
 */
typedef enum bcm_error_e
{
    BCM_E_NONE         = 0, //_SHR_E_NONE, 
    BCM_E_INTERNAL     = -1, //_SHR_E_INTERNAL, 
    BCM_E_MEMORY       = -2, //_SHR_E_MEMORY, 
    BCM_E_UNIT         = -3, //_SHR_E_UNIT, 
    BCM_E_PARAM        = -4, //_SHR_E_PARAM, 
    BCM_E_EMPTY        = -5, //_SHR_E_EMPTY, 
    BCM_E_FULL         = -6, //_SHR_E_FULL, 
    BCM_E_NOT_FOUND    = -7, //_SHR_E_NOT_FOUND, 
    BCM_E_EXISTS       = -8, //_SHR_E_EXISTS, 
    BCM_E_TIMEOUT      = -9, //_SHR_E_TIMEOUT, 
    BCM_E_BUSY         = -10, //_SHR_E_BUSY, 
    BCM_E_FAIL         = -11, //_SHR_E_FAIL, 
    BCM_E_DISABLED     = -12, //_SHR_E_DISABLED, 
    BCM_E_BADID        = -13, //_SHR_E_BADID, 
    BCM_E_RESOURCE     = -14, //_SHR_E_RESOURCE, 
    BCM_E_CONFIG       = -15, //_SHR_E_CONFIG, 
    BCM_E_UNAVAIL      = -16, //_SHR_E_UNAVAIL, 
    BCM_E_INIT         = -17, //_SHR_E_INIT, 
    BCM_E_PORT         = -18, //_SHR_E_PORT, 
    BCM_E_IO           = -19, //_SHR_E_IO, 
    BCM_E_ACCESS       = -20, //_SHR_E_ACCESS, 
    BCM_E_NO_HANDLER   = -21, //_SHR_E_NO_HANDLER, 
    BCM_E_PARTIAL      = -22, //_SHR_E_PARTIAL 
} bcm_error_t;

#define SOC_E_NONE		BCM_E_NONE
#define SOC_E_PARAM		BCM_E_PARAM
#define SOC_E_MEMORY	BCM_E_MEMORY
#define SOC_E_UNAVAIL	BCM_E_UNAVAIL

#define _SHR_E_SUCCESS(rv)              ((rv) >= 0)
#define _SHR_E_FAILURE(rv)              ((rv) < 0)

#define SOC_FAILURE(rv)         _SHR_E_FAILURE(rv)

#define BCM_SUCCESS(rv)         \
    _SHR_E_SUCCESS(rv) 

#define BCM_FAILURE(rv)         \
    _SHR_E_FAILURE(rv) 



#endif  //MACSEC_TYPES_H

