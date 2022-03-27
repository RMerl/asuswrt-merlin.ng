/*
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2010 Instituto Nokia de Tecnologia - INdT
 *  Copyright (C) 2010 ST-Ericsson SA
 *
 *  Author: Marek Skowron <marek.skowron@tieto.com> for ST-Ericsson.
 *  Author: Waldemar Rymarkiewicz <waldemar.rymarkiewicz@tieto.com>
 *          for ST-Ericsson.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include <stdint.h>
#include <glib.h>

#ifdef SAP_DEBUG
#define SAP_VDBG(fmt, arg...) DBG(fmt, arg)
#else
#define SAP_VDBG(fmt...)
#endif

#define SAP_VERSION 0x0101

/* Connection Status - SAP v1.1 section 5.2.2 */
enum sap_status {
	SAP_STATUS_OK				= 0x00,
	SAP_STATUS_CONNECTION_FAILED		= 0x01,
	SAP_STATUS_MAX_MSG_SIZE_NOT_SUPPORTED	= 0x02,
	SAP_STATUS_MAX_MSG_SIZE_TOO_SMALL	= 0x03,
	SAP_STATUS_OK_ONGOING_CALL		= 0x04
};

/* Disconnection Type - SAP v1.1 section 5.2.3 */
enum sap_disconnection_type {
	SAP_DISCONNECTION_TYPE_GRACEFUL		= 0x00,
	SAP_DISCONNECTION_TYPE_IMMEDIATE	= 0x01
};

/* Result codes - SAP v1.1 section 5.2.4 */
enum sap_result {
	SAP_RESULT_OK			= 0x00,
	SAP_RESULT_ERROR_NO_REASON	= 0x01,
	SAP_RESULT_ERROR_NOT_ACCESSIBLE	= 0x02,
	SAP_RESULT_ERROR_POWERED_OFF	= 0x03,
	SAP_RESULT_ERROR_CARD_REMOVED	= 0x04,
	SAP_RESULT_ERROR_POWERED_ON	= 0x05,
	SAP_RESULT_ERROR_NO_DATA	= 0x06,
	SAP_RESULT_NOT_SUPPORTED	= 0x07
};

/* Status Change - SAP v1.1 section 5.2.8 */
enum sap_status_change {
	SAP_STATUS_CHANGE_UNKNOWN_ERROR		= 0x00,
	SAP_STATUS_CHANGE_CARD_RESET		= 0x01,
	SAP_STATUS_CHANGE_CARD_NOT_ACCESSIBLE	= 0x02,
	SAP_STATUS_CHANGE_CARD_REMOVED		= 0x03,
	SAP_STATUS_CHANGE_CARD_INSERTED		= 0x04,
	SAP_STATUS_CHANGE_CARD_RECOVERED	= 0x05
};

/* Message format - SAP v1.1 section 5.1 */
struct sap_parameter {
	uint8_t id;
	uint8_t reserved;
	uint16_t len;
	uint8_t val[0];
	/*
	 * Padding bytes 0-3 bytes
	 */
} __attribute__((packed));

struct sap_message {
	uint8_t id;
	uint8_t nparam;
	uint16_t reserved;
	struct sap_parameter param[0];
} __attribute__((packed));

#define SAP_BUF_SIZE		512
#define SAP_MSG_HEADER_SIZE	4

enum sap_protocol {
	SAP_CONNECT_REQ		= 0x00,
	SAP_CONNECT_RESP	= 0x01,
	SAP_DISCONNECT_REQ	= 0x02,
	SAP_DISCONNECT_RESP	= 0x03,
	SAP_DISCONNECT_IND	= 0x04,
	SAP_TRANSFER_APDU_REQ	= 0x05,
	SAP_TRANSFER_APDU_RESP	= 0x06,
	SAP_TRANSFER_ATR_REQ	= 0x07,
	SAP_TRANSFER_ATR_RESP	= 0x08,
	SAP_POWER_SIM_OFF_REQ	= 0x09,
	SAP_POWER_SIM_OFF_RESP	= 0x0A,
	SAP_POWER_SIM_ON_REQ	= 0x0B,
	SAP_POWER_SIM_ON_RESP	= 0x0C,
	SAP_RESET_SIM_REQ	= 0x0D,
	SAP_RESET_SIM_RESP	= 0x0E,
	SAP_TRANSFER_CARD_READER_STATUS_REQ	= 0x0F,
	SAP_TRANSFER_CARD_READER_STATUS_RESP	= 0x10,
	SAP_STATUS_IND	= 0x11,
	SAP_ERROR_RESP	= 0x12,
	SAP_SET_TRANSPORT_PROTOCOL_REQ	= 0x13,
	SAP_SET_TRANSPORT_PROTOCOL_RESP	= 0x14
};

/* Parameters Ids - SAP 1.1 section 5.2 */
enum sap_param_id {
	SAP_PARAM_ID_MAX_MSG_SIZE	= 0x00,
	SAP_PARAM_ID_CONN_STATUS	= 0x01,
	SAP_PARAM_ID_RESULT_CODE	= 0x02,
	SAP_PARAM_ID_DISCONNECT_IND	= 0x03,
	SAP_PARAM_ID_COMMAND_APDU	= 0x04,
	SAP_PARAM_ID_COMMAND_APDU7816	= 0x10,
	SAP_PARAM_ID_RESPONSE_APDU	= 0x05,
	SAP_PARAM_ID_ATR		= 0x06,
	SAP_PARAM_ID_CARD_READER_STATUS	= 0x07,
	SAP_PARAM_ID_STATUS_CHANGE	= 0x08,
	SAP_PARAM_ID_TRANSPORT_PROTOCOL	= 0x09
};

#define SAP_PARAM_ID_MAX_MSG_SIZE_LEN		0x02
#define SAP_PARAM_ID_CONN_STATUS_LEN		0x01
#define SAP_PARAM_ID_RESULT_CODE_LEN		0x01
#define SAP_PARAM_ID_DISCONNECT_IND_LEN		0x01
#define SAP_PARAM_ID_CARD_READER_STATUS_LEN	0x01
#define SAP_PARAM_ID_STATUS_CHANGE_LEN		0x01
#define SAP_PARAM_ID_TRANSPORT_PROTO_LEN	0x01

/* Transport Protocol - SAP v1.1 section 5.2.9 */
enum sap_transport_protocol {
	SAP_TRANSPORT_PROTOCOL_T0 = 0x00,
	SAP_TRANSPORT_PROTOCOL_T1 = 0x01
};

/*SAP driver init and exit routines. Implemented by sap-*.c */
int sap_init(void);
void sap_exit(void);

/* SAP requests implemented by sap-*.c */
void sap_connect_req(void *sap_device, uint16_t maxmsgsize);
void sap_disconnect_req(void *sap_device, uint8_t linkloss);
void sap_transfer_apdu_req(void *sap_device, struct sap_parameter *param);
void sap_transfer_atr_req(void *sap_device);
void sap_power_sim_off_req(void *sap_device);
void sap_power_sim_on_req(void *sap_device);
void sap_reset_sim_req(void *sap_device);
void sap_transfer_card_reader_status_req(void *sap_device);
void sap_set_transport_protocol_req(void *sap_device,
					struct sap_parameter *param);

/*SAP responses to SAP requests. Implemented by server.c */
int sap_connect_rsp(void *sap_device, uint8_t status);
int sap_disconnect_rsp(void *sap_device);
int sap_transfer_apdu_rsp(void *sap_device, uint8_t result,
				uint8_t *sap_apdu_resp, uint16_t length);
int sap_transfer_atr_rsp(void *sap_device, uint8_t result,
				uint8_t *sap_atr, uint16_t length);
int sap_power_sim_off_rsp(void *sap_device, uint8_t result);
int sap_power_sim_on_rsp(void *sap_device, uint8_t result);
int sap_reset_sim_rsp(void *sap_device, uint8_t result);
int sap_transfer_card_reader_status_rsp(void *sap_device, uint8_t result,
						uint8_t status);
int sap_transport_protocol_rsp(void *sap_device, uint8_t result);

/* Event indication. Implemented by server.c*/
int sap_status_ind(void *sap_device, uint8_t status_change);
int sap_disconnect_ind(void *sap_device, uint8_t disc_type);
