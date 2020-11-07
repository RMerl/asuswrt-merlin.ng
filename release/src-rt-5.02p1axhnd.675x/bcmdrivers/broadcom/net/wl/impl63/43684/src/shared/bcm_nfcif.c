/*
 * API Implementation to communicate with NFC chip using
 * debug UART APIs.
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
 * $Id: bcm_nfcif.c 596126 2015-10-29 19:53:48Z $
 */

#include <bcm_cfg.h>
#include <typedefs.h>
#include <osl.h>
#include <bcmutils.h>
#include <bcmdevs.h>
#include <bcm_nfcif.h>
#include <hnd_cons.h>

#ifdef BCMDBG_ERR
#define NFCIF_ERR(args) printf args
#else
#define NFCIF_ERR(args)
#endif  /* BCMDBG_ERR */

#define	NFC_MAX_PAYLOAD_LENGTH	60

/* NFC Receive states */
#define	NFC_RX_ST_RECV_START_OF_PKT	1
#define NFC_RX_ST_RECV_1ST_HDR_BYTE	2
#define NFC_RX_ST_RECV_2ND_HDR_BYTE	3
#define	NFC_RX_ST_RECV_DATA		4

typedef struct _nfc_rx_ctxt {
	uint8	rx_state;
	uint8	payload_length;
	uint8	eop;
	uint8	pkt_type;
	uint8	payload[NFC_MAX_PAYLOAD_LENGTH];
	uint8	bytes_rxd;

	/* rx callback ctx */
	rx_cb_t	rx_cb;
	void	*rx_cb_param;
} nfc_rx_ctxt_t;
nfc_rx_ctxt_t	rx_ctxt;

static void bcm_nfcif_reset_receiver(void);
static void bcm_nfcif_recvd_from_nfc(void *ctx, uint8 c);

/* defines for status messages */
#define	NFC_SUCCESS		 0
#define NFC_BUF_INVALID		-1
#define NFC_BUF_LEN_INVALID	-2

/* defines for custom SLIP pkt */
#define NFC_START_OF_PKT	0xC0

/* SLIP header description
 * 8-bit payload length
 *	[0 - 60] - valid range
 * 1-bit end of packet
 *	0b - more frgment(s) to follow
 *	1b - last frgment
 * 2-bit pkt_type
 *	00b - Ack pkt
 *	01b - WiFi pkt
 *	10b - NFC host pkt
 *	11b - Link Control pkt
 * 5-bit reserved
 */
#define	SLIP_SOP_SIZE	1
#define	SLIP_HDR_SIZE	2
#define SLIP_HDR_PAYLOAD_LEN_SHIFT	0
#define SLIP_HDR_PAYLOAD_LEN_MASK	(0xFF)
#define	SLIP_HDR_EOP_SHIFT		7
#define	SLIP_HDR_EOP_MASK		(0x01)
#define	SLIP_HDR_PKT_TYPE_SHIFT		5
#define	SLIP_HDR_PKT_TYPE_MASK		(0x03)

#define SET_SLIP_HDR_FIELDS(eop, pt)	(((eop & SLIP_HDR_EOP_MASK) << SLIP_HDR_EOP_SHIFT) | \
					 ((pt & SLIP_HDR_PKT_TYPE_MASK) << SLIP_HDR_PKT_TYPE_SHIFT))

void
bcm_nfcif_send_host_cmd(char *buf, uint16 len)
{
	bcm_nfcif_send_to_nfc(buf, len, SLIP_HDR_HOST_PKT);
}
int
bcm_nfcif_send_to_nfc(char *buf, uint16 len, uint8 pkt_type)
{
	uint8	slip_hdr[2];
	uint8	slip_pkt[64];
	uint8	slip_idx = 0;

	if (!buf) {
		return NFC_BUF_INVALID;
	}

	if (len > NFC_MAX_PAYLOAD_LENGTH) {
		return NFC_BUF_LEN_INVALID;
	}

	/* buffer is valid; process to send to NFC */
	slip_pkt[slip_idx] = NFC_START_OF_PKT;
	slip_idx++;

	/* prepare slip header */
	slip_hdr[0] = (uint8)(len & 0xFF);
	slip_hdr[1] = SET_SLIP_HDR_FIELDS(1, pkt_type);

	bcopy(&slip_hdr, &slip_pkt[slip_idx], SLIP_HDR_SIZE);
	slip_idx += SLIP_HDR_SIZE;

	/* copy the payload */
	bcopy(buf, &slip_pkt[slip_idx], len);

	hndrte_uart_tx(slip_pkt, (len + SLIP_HDR_SIZE + 1));

	return NFC_SUCCESS;
}

static void
bcm_nfcif_reset_receiver(void)
{
	rx_ctxt.rx_state = NFC_RX_ST_RECV_START_OF_PKT;
	rx_ctxt.payload_length = 0;
	rx_ctxt.bytes_rxd = 0;
}

/*
 *	NFC receive function
 */
uint8
bcm_nfcif_rx(char *buf, uint8 count)
{
	int bytes_to_copy = 0;

	if (rx_ctxt.bytes_rxd > 0) {
		bytes_to_copy = (count < rx_ctxt.bytes_rxd)? count :
			rx_ctxt.bytes_rxd;
		bcopy(rx_ctxt.payload, buf, bytes_to_copy);
	}
	bcm_nfcif_reset_receiver();

	return bytes_to_copy;
}

/*
 *	NFC receive state machine
 */
void
bcm_nfcif_recvd_from_nfc(void *ctx, uint8 c)
{
	BCM_REFERENCE(ctx);

	switch (rx_ctxt.rx_state) {
		case NFC_RX_ST_RECV_START_OF_PKT:
			if (c == NFC_START_OF_PKT)
				rx_ctxt.rx_state = NFC_RX_ST_RECV_1ST_HDR_BYTE;
			break;
		case NFC_RX_ST_RECV_1ST_HDR_BYTE:
			rx_ctxt.payload_length = c;
			if (rx_ctxt.payload_length > NFC_MAX_PAYLOAD_LENGTH) {
				NFCIF_ERR(("**** NFC sends more bytes than agreed \n"));
			}
			rx_ctxt.rx_state = NFC_RX_ST_RECV_2ND_HDR_BYTE;
			break;
		case NFC_RX_ST_RECV_2ND_HDR_BYTE:
			rx_ctxt.eop = ((c >> SLIP_HDR_EOP_SHIFT) & SLIP_HDR_EOP_MASK);
			rx_ctxt.pkt_type = ((c >> SLIP_HDR_PKT_TYPE_SHIFT) &
				SLIP_HDR_PKT_TYPE_MASK);
			rx_ctxt.rx_state = NFC_RX_ST_RECV_DATA;
			break;
		case NFC_RX_ST_RECV_DATA:
			if (rx_ctxt.payload_length < NFC_MAX_PAYLOAD_LENGTH) {
				rx_ctxt.payload[rx_ctxt.bytes_rxd] = c;
				rx_ctxt.bytes_rxd++;
			}

			if (rx_ctxt.bytes_rxd == rx_ctxt.payload_length) {
				if (rx_ctxt.pkt_type == SLIP_HDR_WIFI_PKT) {
					/* pass the payload to wl */
					if (rx_ctxt.rx_cb) {
						rx_ctxt.rx_cb(rx_ctxt.rx_cb_param,
							(char *)rx_ctxt.payload,
							rx_ctxt.payload_length);
					}
				} else if (rx_ctxt.pkt_type == SLIP_HDR_HOST_PKT) {
					/* may need different callback
					 * as this pkt is of host's(dhd) interest
					 */
				}
				bcm_nfcif_reset_receiver();
			}
			break;
		default:
			/* receiver state mc messed up */
			break;
	}
}

void
bcm_nfcif_init(rx_cb_t rx_cb, void *param)
{

#define	HCI_RST_CMD_LEN	4
	char	hci_reset[HCI_RST_CMD_LEN] = {0x01, 0x03, 0x0C, 0x00};
#define	NCI_XTAL_IDX_LEN	7
	char	nci_xtal_idx[NCI_XTAL_IDX_LEN] =
		{0x10, 0x2F, 0x1D, 0x03, 0x05, 0x00, 0x00};

#define	NFC_SPD_GET_VER_CMD_LEN		4
	char nfc_spd_get_ver[NFC_SPD_GET_VER_CMD_LEN] = { 0x10, 0x2F, 0x2D, 0x00};

#define	RESET_CMD_LEN			5
	char reset_cmd[RESET_CMD_LEN] = { 0x10, 0x20, 0x00, 0x01, 0x01};

#define	INIT_CMD_LEN			4
	char init_cmd[INIT_CMD_LEN] = { 0x10, 0x20, 0x01, 0x00};

#define	SUP_32BIT_ARM_POKE_LEN		14
	char sup_32bit_arm_poke[SUP_32BIT_ARM_POKE_LEN] =
		{ 0x01, 0x0C, 0xFC, 0x0A, 0x08, 0x08, 0xD0, 0x15, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00};

#define	DISC_MAP_CMD_LEN		14
	char disc_map_cmd[DISC_MAP_CMD_LEN] =
		{ 0x10, 0x21, 0x00, 0x0A, 0x03, 0x04, 0x02, 0x02, 0x02, 0x02, 0x01,
		0x05, 0x02, 0x03};

#define	SET_CFG_CMD_1_LEN		8
	char set_cfg_cmd1[SET_CFG_CMD_1_LEN] =
		{ 0x10, 0x20, 0x02, 0x04, 0x01, 0xA5, 0x01, 0x01};

#define	SET_CFG_CMD_2_LEN		9
	char set_cfg_cmd2[SET_CFG_CMD_2_LEN] =
		{ 0x10, 0x20, 0x02, 0x05, 0x01, 0xC2, 0x02, 0x60, 0x08};

#define	SET_CFG_CMD_3_LEN		8
	char set_cfg_cmd3[SET_CFG_CMD_3_LEN] =
		{ 0x10, 0x20, 0x02, 0x04, 0x01, 0x5B, 0x01, 0x03};

#define	SET_CFG_CMD_4_LEN		18
	char set_cfg_cmd4[SET_CFG_CMD_4_LEN] =
		{ 0x10, 0x20, 0x02, 0x0E, 0x01, 0xD6, 0x0B, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x5F};

#define	SET_FWFSM_CMD_LEN		5
	char set_fwfsm_cmd[SET_FWFSM_CMD_LEN] = { 0x10, 0x2F, 0x06, 0x01, 0x01};

#define	DISC_NFCEE_CMD_LEN		5
	char disc_nfcee_cmd[DISC_NFCEE_CMD_LEN] = { 0x10, 0x22, 0x00, 0x01, 0x01};

#define	CREATE_CONN_CMD			10
	char create_conn_cmd[CREATE_CONN_CMD] =
		{ 0x10, 0x20, 0x04, 0x06, 0x03, 0x01, 0x01, 0x02, 0xF2, 0x01};

#define	DATA_MSG1_SEND_LEN		6
	char data_msg1_send[DATA_MSG1_SEND_LEN] = { 0x10, 0x01, 0x00, 0x02, 0x81, 0x03};

#define	DATA_MSG2_SEND_LEN		8
	char data_msg2_send[DATA_MSG2_SEND_LEN] = { 0x10, 0x01, 0x00, 0x04, 0x81, 0x14, 0x12, 0x34};
#define	DATA_MSG3_SEND_LEN		6
	char data_msg3_send[DATA_MSG3_SEND_LEN] = { 0x10, 0x01, 0x00, 0x02, 0x81, 0x03};
#define	DATA_MSG4_SEND_LEN		9
	char data_msg4_send[DATA_MSG4_SEND_LEN] =
		{ 0x10, 0x01, 0x00, 0x05, 0x81, 0x01, 0x03, 0x02, 0x03};
#define	DATA_MSG5_SEND_LEN		6
	char data_msg5_send[DATA_MSG5_SEND_LEN] = { 0x10, 0x01, 0x00, 0x02, 0x81, 0x80};
#define	DATA_MSG6_SEND_LEN		6
	char data_msg6_send[DATA_MSG6_SEND_LEN] = { 0x10, 0x01, 0x00, 0x02, 0x85, 0x80};
#define	DATA_MSG7_SEND_LEN		9
	char data_msg7_send[DATA_MSG7_SEND_LEN] =
		{ 0x10, 0x01, 0x00, 0x05, 0x81, 0x10, 0xC3, 0x00, 0xC3};
#define	DATA_MSG8_SEND_LEN		6
	char data_msg8_send[DATA_MSG8_SEND_LEN] = { 0x10, 0x01, 0x00, 0x02, 0x86, 0x03};
#define	REMOTE_NFC_CMD_LEN	21
	char remote_nfc_cmd[REMOTE_NFC_CMD_LEN] =
		{ 0x00, 0xA4, 0x04, 0x00, 0x0E, 0x32, 0x50, 0x41, 0x59, 0x2E, 0x53,
		0x59, 0x53, 0x2E, 0x44, 0x44, 0x46, 0x30, 0x31, 0x00, 0x00};

	bcm_nfcif_send_to_nfc(hci_reset,  HCI_RST_CMD_LEN, SLIP_HDR_HOST_PKT);
	OSL_DELAY(260000); /* 100ms delay */
	bcm_nfcif_send_to_nfc(nci_xtal_idx, NCI_XTAL_IDX_LEN, SLIP_HDR_HOST_PKT);
	OSL_DELAY(32000);
	bcm_nfcif_send_host_cmd(nfc_spd_get_ver, NFC_SPD_GET_VER_CMD_LEN);
	OSL_DELAY(135000);
	bcm_nfcif_send_host_cmd(reset_cmd, RESET_CMD_LEN);
	OSL_DELAY(31000);
	bcm_nfcif_send_host_cmd(init_cmd, INIT_CMD_LEN);
	OSL_DELAY(53000);
	bcm_nfcif_send_host_cmd(sup_32bit_arm_poke, SUP_32BIT_ARM_POKE_LEN);
	OSL_DELAY(83000);
	bcm_nfcif_send_host_cmd(disc_map_cmd, DISC_MAP_CMD_LEN);
	OSL_DELAY(55000);
	bcm_nfcif_send_host_cmd(set_cfg_cmd1, SET_CFG_CMD_1_LEN);
	OSL_DELAY(61000);
	bcm_nfcif_send_host_cmd(set_cfg_cmd2, SET_CFG_CMD_2_LEN);
	OSL_DELAY(60000);
	bcm_nfcif_send_host_cmd(set_cfg_cmd3, SET_CFG_CMD_3_LEN);
	OSL_DELAY(90000);
	bcm_nfcif_send_host_cmd(set_cfg_cmd4, SET_CFG_CMD_4_LEN);
	OSL_DELAY(77000);
	bcm_nfcif_send_host_cmd(set_fwfsm_cmd, SET_FWFSM_CMD_LEN);
	OSL_DELAY(30000);
	bcm_nfcif_send_host_cmd(disc_nfcee_cmd, DISC_NFCEE_CMD_LEN);
	OSL_DELAY(95000);
	bcm_nfcif_send_host_cmd(create_conn_cmd, CREATE_CONN_CMD);
	OSL_DELAY(53000);
	bcm_nfcif_send_host_cmd(data_msg1_send, DATA_MSG1_SEND_LEN);
	OSL_DELAY(60000);
	bcm_nfcif_send_host_cmd(data_msg2_send, DATA_MSG2_SEND_LEN);
	OSL_DELAY(60000);
	bcm_nfcif_send_host_cmd(data_msg3_send, DATA_MSG3_SEND_LEN);
	OSL_DELAY(90000);
	bcm_nfcif_send_host_cmd(data_msg4_send, DATA_MSG4_SEND_LEN);
	OSL_DELAY(170000);
	bcm_nfcif_send_host_cmd(data_msg5_send, DATA_MSG5_SEND_LEN);
	OSL_DELAY(170000);
	bcm_nfcif_send_host_cmd(data_msg6_send, DATA_MSG6_SEND_LEN);
	OSL_DELAY(170000);
	bcm_nfcif_send_host_cmd(data_msg7_send, DATA_MSG7_SEND_LEN);
	OSL_DELAY(170000);
	bcm_nfcif_send_host_cmd(data_msg8_send, DATA_MSG8_SEND_LEN);
	OSL_DELAY(170000);
	bcm_nfcif_send_to_nfc(remote_nfc_cmd, REMOTE_NFC_CMD_LEN, SLIP_HDR_WIFI_PKT);

	bcm_nfcif_reset_receiver();
	rx_ctxt.rx_cb = rx_cb;
	rx_ctxt.rx_cb_param = param;

	hndrte_register_uart_rx_cb(&rx_ctxt, bcm_nfcif_recvd_from_nfc);
}
