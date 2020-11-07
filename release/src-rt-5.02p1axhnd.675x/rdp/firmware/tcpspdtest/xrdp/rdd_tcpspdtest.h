/*
* <:copyright-BRCM:2014:proprietary:standard
* 
*    Copyright (c) 2014 Broadcom 
*    All Rights Reserved
* 
*  This program is the proprietary software of Broadcom and/or its
*  licensors, and may only be used, duplicated, modified or distributed pursuant
*  to the terms and conditions of a separate, written license agreement executed
*  between you and Broadcom (an "Authorized License").  Except as set forth in
*  an Authorized License, Broadcom grants no license (express or implied), right
*  to use, or waiver of any kind with respect to the Software, and Broadcom
*  expressly reserves all rights in and to the Software and all intellectual
*  property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU HAVE
*  NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY
*  BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
* 
*  Except as expressly set forth in the Authorized License,
* 
*  1. This program, including its structure, sequence and organization,
*     constitutes the valuable trade secrets of Broadcom, and you shall use
*     all reasonable efforts to protect the confidentiality thereof, and to
*     use this information only in connection with your use of Broadcom
*     integrated circuit products.
* 
*  2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
*     AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
*     WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
*     RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND
*     ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT,
*     FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR
*     COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE
*     TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF USE OR
*     PERFORMANCE OF THE SOFTWARE.
* 
*  3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR
*     ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
*     INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY
*     WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
*     IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES;
*     OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE
*     SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS
*     SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY
*     LIMITED REMEDY.
* :>
*/


/*
 * rdd_tcpspdtest.h
 */

#ifndef _RDD_TCPSPDTEST_H_
#define _RDD_TCPSPDTEST_H_

#include "rdd.h"
#include "rdd_ag_tcpspdtest.h"
#include "rdd_ag_pktgen_tx.h"

#define TCPSPDTEST_NUM_OF_DATA_PKTS PKTGEN_TX_NUM_OF_DATA_PKTS

bdmf_error_t rdd_tcpspdtest_engine_global_info_set(RDD_TCPSPDTEST_ENGINE_GLOBAL_INFO_DTS *global_info);
bdmf_error_t rdd_tcpspdtest_engine_global_info_get(RDD_TCPSPDTEST_ENGINE_GLOBAL_INFO_DTS *global_info);
bdmf_error_t rdd_tcpspdtest_engine_conn_info_set(RDD_TCPSPDTEST_ENGINE_CONN_INFO_DTS *conn_info,
    RDD_PKTGEN_TX_STREAM_ENTRY_DTS *pktgen_tx_stream, uint8_t stream_idx);
bdmf_error_t rdd_tcpspdtest_engine_conn_info_get(RDD_TCPSPDTEST_ENGINE_CONN_INFO_DTS *conn_info,
    RDD_PKTGEN_TX_STREAM_ENTRY_DTS *pktgen_tx_stream, uint8_t stream_idx);
bdmf_error_t rdd_tcpspdtest_engine_tcb_set(RDD_TCPSPDTEST_ENGINE_TCB_DTS *params, uint8_t stream_idx);
bdmf_error_t rdd_tcpspdtest_engine_tcb_get(RDD_TCPSPDTEST_ENGINE_TCB_DTS *params, uint8_t stream_idx);
bdmf_error_t rdd_tcpspdtest_engine_ref_pkt_hdr_set(uint16_t size, uint16_t offset, uint8_t *hdr, uint8_t stream_idx);
bdmf_error_t rdd_tcpspdtest_engine_ref_pkt_hdr_get(uint16_t *size, uint16_t *offset, uint8_t *hdr, uint8_t stream_idx, uint16_t read_size);
bdmf_error_t rdd_tcpspdtest_engine_pkt_drop_set(RDD_TCPSPDTEST_ENGINE_PKT_DROP_DTS *params, uint8_t stream_idx);
bdmf_error_t rdd_tcpspdtest_engine_pkt_drop_get(RDD_TCPSPDTEST_ENGINE_PKT_DROP_DTS *params, uint8_t stream_idx);
bdmf_error_t rdd_tcpspdtest_engine_start_upload_tx(uint8_t stream_idx, uint8_t num_active_streams);
bdmf_error_t rdd_tcpspdtest_engine_stop_upload_tx(uint8_t stream_idx, uint8_t num_active_streams);

#endif /* _RDD_TCPSPDTEST_H_ */
