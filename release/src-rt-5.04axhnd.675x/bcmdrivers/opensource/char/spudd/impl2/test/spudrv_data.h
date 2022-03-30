/*
<:copyright-BRCM:2007:DUAL/GPL:standard

   Copyright (c) 2007 Broadcom 
   All Rights Reserved

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
