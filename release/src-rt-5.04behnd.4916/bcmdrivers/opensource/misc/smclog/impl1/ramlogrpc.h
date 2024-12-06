/* SPDX-License-Identifier: GPL-2.0+
 *
 * Copyright 2022 Broadcom Ltd.
 */
/*
   <:copyright-BRCM:2022:DUAL/GPL:standard

   Copyright (c) 2022 Broadcom 
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

/****************************************************************************
 *
 * Filename:    ramlogrpc.h
 *
 ****************************************************************************
 *
 * Description: ramlog RPC definitions
 *
 ****************************************************************************/

/**
 * @file    ramlogrpc.h
 * @brief   ramlog RPC definitions
 */

#ifndef __RAMLOGRPC_H_
#define __RAMLOGRPC_H_

#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/wait.h>
#include <linux/dma-mapping.h>

#define	RAMLOG_ALIGNED(x)		ALIGN((x), 8)

#define	RAMLOG_FIFO_COUNT		(32 * 1024)
#define	RAMLOG_MSG_BUFFER_SIZE		RAMLOG_ALIGNED(32 * 1024 * 128)

#define	RAMLOG_NULL_INDEX		(~0ULL)

#define	RAMLOGFLAG_SEVERITY_MASK	0x0007
#define	RAMLOGFLAG_SEVERITY_LENGTH	3
#define	RAMLOGFLAG_SEVERITY_SHIFT	0

#define	RAMLOGFLAG_SEVERITY_NONE	0x0000
#define	RAMLOGFLAG_SEVERITY_DEEP_DEBUG	0x0006
#define	RAMLOGFLAG_SEVERITY_DEBUG	0x0005
#define	RAMLOGFLAG_SEVERITY_MESSAGE	0x0004
#define	RAMLOGFLAG_SEVERITY_NOTICE	0x0003
#define	RAMLOGFLAG_SEVERITY_WARNING	0x0002
#define	RAMLOGFLAG_SEVERITY_ERROR	0x0001

#define	INVALID_RAMLOG_START_INDEX	0xffffffff
#define	RAMLOG_HEAD_START_INDEX		0xfffffffe
#define	RAMLOG_TAIL_START_INDEX		0xfffffffd

#define	CRYPTO_AES_IV_SIZE		16

/**
 * This is an encrypted log buffer header
 */
struct smc_ramlog_hdr {
    uint32_t magic;
    uint32_t length;
    uint8_t aes_iv[CRYPTO_AES_IV_SIZE];
    uint16_t aes_log_saltindex;
    uint8_t aes_log_keyindex;
    uint8_t hdr_ver;
    uint32_t reserved;
};

/**
 * This structure represents coordinates of SMC ramlog RPC tunnel
 */
struct ramlog_rpc_data {
    struct device *dev;
    int tunnel;
    char tunnel_name[32];
    struct semaphore sem;
};

/**
 * This routine registers all required resources to connect to SMC ramlog DQM
 *
 * @param[out]  ramlog              ramlog coordinates
 * @param[in]   dev                 a device pointer
 * @param[in]   tunnel_name         a tunnel name
 * @return                          zero on success or a negative error code
 */
int register_ramlog_rpc(struct ramlog_rpc_data *ramlog, struct device *dev,
    const char *tunnel_name);

/**
 * This routine registers all required resources to connect to SMC ramlog DQM
 * from a platform device
 *
 * @param[out]  ramlog              ramlog coordinates
 * @param[in]   pdev                a platform device pointer
 * @return                          zero on success or a negative error code
 */
int register_ramlog_rpc_from_platform_device(struct ramlog_rpc_data *ramlog,
    struct platform_device *pdev);

/**
 * This routine releases all required resources to connect to SMC ramlog DQM
 *
 * @param[out]  ramlog              ramlog coordinates
 */
void release_ramlog_rpc(struct ramlog_rpc_data *ramlog);

/**
 * This routine pulls encrypted records from ramlog into a buffer
 *
 * @param[out]  encrypted_dma_addr  store encrypted data there
 * @param[in]   maxcount            max. number of bytes that may be put
 *                                  into the buffer
 * @param[in]   ramlog              ramlog coordinates
 * @param[out]  count_ptr           if not null, a number of records actually
 *                                  pulled is stored there
 * @param[out]  update_ptr          if not null, an update stamp is stored
 *                                  here, but only when the last record was
 *                                  pulled
 * @param[in]   start_index         a position to start or one of the special
 *                                  values:
 *                                  INVALID_RAMLOG_START_INDEX  start at
 *                                                              the head
 *                                  RAMLOG_HEAD_START_INDEX     start at
 *                                                              the head
 *                                  RAMLOG_TAIL_START_INDEX     pull
 *                                                              maxcount most
 *                                                              recent records
 * @return                          an index of the next start position or
 *                                  INVALID_RAMLOG_START_INDEX
 */
uint32_t get_encrypted_ramlog_entries(dma_addr_t encrypted_dma_addr,
    uint32_t maxbufsize, struct ramlog_rpc_data *ramlog,
    uint32_t *count_ptr, uint32_t *update_ptr,
    uint32_t start_index);

/**
 * This routine makes ramlog empty
 *
 * @param[out]  ramlog              ramlog coordinates
 * @return                          zero or an error code
 */
int clear_ramlog(struct ramlog_rpc_data *ramlog);

/**
 * This routine adds another entry to the ramlog
 *
 * @param[out]  ramlog              ramlog coordinates
 * @param[in]   severity            a severity of the message
 * @param[in]   length              length of the message buffer
 *                                  (string length + 1)
 * @param[in]   message_dma_addr    contents of the actual message to report
 * @return                          zero on success or an error code
 */
int add_to_ramlog(struct ramlog_rpc_data *ramlog, unsigned int severity,
    unsigned int length, dma_addr_t message_dma_addr);

/**
 * This routine sets the current value of global reporting level.
 *
 * @param[out]  ramlog              ramlog coordinates
 * @param[in]   level               a new value of ramlog level
 * @param[in]   length              length of the component name buffer
 *                                  (string length + 1)
 * @param[in]   source_dma_addr     a name of the component for which level
 *                                  should be customized. if set to "*" or
 *                                  "!" all custom levels are removed and
 *                                  the global ramlog level is set.
 * @return                          zero on success or an error code
 */
int set_ramlog_level(struct ramlog_rpc_data *ramlog, unsigned int level,
    unsigned int length, dma_addr_t source_dma_addr);

/**
 * This routine sets up an UART flag for ramlog
 *
 * @param[out]  ramlog              ramlog coordinates
 * @param[in]   flag                a new UART flag
 * @return                          zero on success or an error code
 */
int set_ramlog_uart_flag(struct ramlog_rpc_data *ramlog, unsigned int flag);

/**
 * This routine returns the current number of entries present in the ramlog
 *
 * @param[in]   ramlog              ramlog coordinates
 * @return                          a number of entries currently sitting
 *                                  in the log
 */
unsigned int get_ramlog_count(struct ramlog_rpc_data *ramlog);

/**
 * This function returns a 32-bit number that represents the last update
 * of the ramlog
 *
 * @param[in]   ramlog              ramlog coordinates
 * @return                          the last update (counter)
 */
uint32_t get_ramlog_last_update(struct ramlog_rpc_data *ramlog);

#endif	/* __RAMLOGRPC_H_ */
