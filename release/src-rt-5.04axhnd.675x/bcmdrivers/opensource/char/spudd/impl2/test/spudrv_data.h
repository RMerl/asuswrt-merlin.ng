/*
<:copyright-BRCM:2007:DUAL/GPL:standard

   Copyright (c) 2007 Broadcom 
   All Rights Reserved

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
 * File Name  : spudrv_data.h
 *
 * Description: This file contains diagnostic vectors to validate
 *              the IPSec SPU.
 *
 * Updates    : 11/16/2007  Pavan Kumar.  Created.
 ***************************************************************************/
#ifndef __SPUDRV_DATA_H__
#define __SPUDRV_DATA_H__

#if defined(CONFIG_BCM_SPU_TEST)

#define AES_MD5   0
#define AES_SHA1  0
#define DES_MD5   0
#define DES_SHA1  1
#define DES3_MD5  0
#define DES3_SHA1  0

#if AES_MD5
#include "spudrv_tx_aes_md5_data.h"
#include "spudrv_rx_aes_md5_data.h"
#define tx_test_pkts    tx_aes_md5_test_pkts
#define rx_templates    rx_aes_md5_test_pkts
#define tx_pkt_len      tx_aes_md5_pkt_len
#define rx_pkt_len      rx_aes_md5_pkt_len
#endif
#if AES_SHA1
#include "spudrv_tx_aes_sha1_data.h"
#include "spudrv_rx_aes_sha1_data.h"
#define tx_test_pkts    tx_aes_sha1_test_pkts
#define rx_templates    rx_aes_sha1_test_pkts
#define tx_pkt_len      tx_aes_sha1_pkt_len
#define rx_pkt_len      rx_aes_sha1_pkt_len
#endif
#if DES_SHA1
#include "spudrv_tx_des_sha1_data.h"
#include "spudrv_rx_des_sha1_data.h"
#define tx_test_pkts    tx_des_sha1_test_pkts
#define rx_templates    rx_des_sha1_test_pkts
#define tx_pkt_len      tx_des_sha1_pkt_len
#define rx_pkt_len      rx_des_sha1_pkt_len
#endif
#if DES_MD5
#include "spudrv_tx_des_md5_data.h"
#include "spudrv_rx_des_md5_data.h"
#define tx_test_pkts    tx_des_md5_test_pkts
#define rx_templates    rx_des_md5_test_pkts
#define tx_pkt_len      tx_des_md5_pkt_len
#define rx_pkt_len      rx_des_md5_pkt_len
#endif
#if DES3_MD5
#include "spudrv_tx_des3_md5_data.h"
#include "spudrv_rx_des3_md5_data.h"
#define tx_test_pkts    tx_des3_md5_test_pkts
#define rx_templates    rx_des3_md5_test_pkts
#define tx_pkt_len      tx_des3_md5_pkt_len
#define rx_pkt_len      rx_des3_md5_pkt_len
#endif
#if DES3_SHA1
#include "spudrv_tx_des3_sha1_data.h"
#include "spudrv_rx_des3_sha1_data.h"
#define tx_test_pkts    tx_des3_sha1_test_pkts
#define rx_templates    rx_des3_sha1_test_pkts
#define tx_pkt_len      tx_des3_sha1_pkt_len
#define rx_pkt_len      rx_des3_sha1_pkt_len
#endif

#define MAX_PKT_ID        100
#define MAX_PKTS_PER_TEST 100


#endif /* CONFIG_BCM_SPU_TEST */
#endif /* __SPUDRV_DATA_H__ */
