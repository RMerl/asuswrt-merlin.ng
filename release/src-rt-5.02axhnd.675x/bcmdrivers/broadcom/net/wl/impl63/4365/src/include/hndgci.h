/*
 * HND GCI core software interface
 *
 * Copyright 2020 Broadcom
 *
 * This program is the proprietary software of Broadcom and/or
 * its licensors, and may only be used, duplicated, modified or distributed
 * pursuant to the terms and conditions of a separate, written license
 * agreement executed between you and Broadcom (an "Authorized License").
 * Except as set forth in an Authorized License, Broadcom grants no license
 * (express or implied), right to use, or waiver of any kind with respect to
 * the Software, and Broadcom expressly reserves all rights in and to the
 * Software and all intellectual property rights therein.  IF YOU HAVE NO
 * AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY
 * WAY, AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF
 * THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1. This program, including its structure, sequence and organization,
 * constitutes the valuable trade secrets of Broadcom, and you shall use
 * all reasonable efforts to protect the confidentiality thereof, and to
 * use this information only in connection with your use of Broadcom
 * integrated circuit products.
 *
 * 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 * "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES,
 * REPRESENTATIONS OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR
 * OTHERWISE, WITH RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY
 * DISCLAIMS ANY AND ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY,
 * NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES,
 * ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR
 * CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING
 * OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL
 * BROADCOM OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL,
 * SPECIAL, INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR
 * IN ANY WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
 * IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii)
 * ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF
 * OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY
 * NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 *
 * APIs to access Plain UART mode functionality of GCI
 *
 * $Id: $
 */

#ifndef _hndgci_h_
#define _hndgci_h_

/* SECI Block operating modes */
#define HND_GCI_PLAIN_UART_MODE		0
#define HND_GCI_SECI_MODE		1
#define HND_GCI_BTSIG_MODE		2
#define HND_GCI_PURE_GPIO_MODE		3

/* Status/Error messages */
#define	HND_GCI_SUCCESS		0
#define	HND_GCI_UNINITIALIZED	-1
#define	HND_GCI_INVALID_PARAM	-2
#define	HND_GCI_INVALID_BUFFER	-3
#define	HND_GCI_TX_INPROGRESS	-4
#define	HND_GCI_NO_SUPPORT	-5
#define	HND_GCI_NO_MEMORY	-6

/* baudrate index */
enum _baudrate_idx {
	GCI_UART_BR_115200,
	GCI_UART_MAX_BR_IDX
};

typedef void (*hndgci_cb_t)(char *buf, uint32 len);
typedef	void (*rx_cb_t)(void *ctx, char *buf, int len);

typedef struct _hndgci_cbs_t {
	void *context;
	void (*rx_cb)(void *ctx, char *buf, int len);
	void (*tx_status)(void *ctx, int status);
} hndgci_cbs_t;

extern int hndgci_init(si_t *sih, osl_t *osh, uint8 seci_mode, uint8 baudrate_idx);
extern void hndgci_deinit(si_t *sih);
extern int hndgci_uart_tx(si_t *sih, void *buf, int len);
extern int hndgci_uart_config_rx_complete(char delim, int len, int timeout,
	rx_cb_t rx_cb, void *ctx);
extern void hndgci_handler_process(uint32 stat, si_t *sih);

/* debug function */
extern int hndgci_uart_rx(si_t *sih, char *buf, int count, int timeout);

#endif /* _hndgci_h_ */
