/*
* <:copyright-BRCM:2013:DUAL/GPL:standard
* 
*    Copyright (c) 2013 Broadcom 
*    All Rights Reserved
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
*/


#ifndef _RDPA_IPSEC_H_
#define _RDPA_IPSEC_H_

/** \addtogroup ipsec Interface
 *
 * @{
 */

#define CPU_RX_IPSEC_LOOPBACK_QUEUE_SIZE  256

/* IPSEC_AUTH_KEY_SIZE_MAX must be equal to RDD_IPSEC_SA_DESC_KEYS_AUTH_KEY_NUMBER */
#define IPSEC_AUTH_KEY_SIZE_MAX     32
/* IPSEC_CRYPT_KEY_SIZE_MAX must be equal to RDD_IPSEC_SA_DESC_KEYS_CRYPT_KEY_NUMBER */
#define IPSEC_CRYPT_KEY_SIZE_MAX    32

/* Bit fields of rdpa_cpu_rx_info_t info.reason_data */
#define IPSEC_ERROR(reason_data)          ((reason_data & 0x700) >> 8)
#define IPSEC_CPU_RX_QUEUE(reason_data)   ((reason_data & 0x03C) >> 2)
#define IPSEC_UPSTREAM_FLAG(reason_data)  ((reason_data & 0x002) >> 1)

/* SA descriptor */
typedef struct {
   uint32_t    spi;
   uint32_t    auth_config;
   uint32_t    crypt_config;
   uint32_t    crypt_config2;
   uint8_t     auth_key[IPSEC_AUTH_KEY_SIZE_MAX];
   uint8_t     crypt_key[IPSEC_CRYPT_KEY_SIZE_MAX];
} rdpa_sa_desc_t;


/** @} end of ipsec Doxygen group */

#endif /* _RDPA_IPSEC_H_ */
