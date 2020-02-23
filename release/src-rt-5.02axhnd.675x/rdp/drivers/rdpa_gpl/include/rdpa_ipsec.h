/*
* <:copyright-BRCM:2013:DUAL/GPL:standard
* 
*    Copyright (c) 2013 Broadcom 
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
