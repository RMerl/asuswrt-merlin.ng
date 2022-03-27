/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2012  Tieto Poland
 *
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
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include "parser.h"

#define PADDING4(x) ((4 - ((x) & 0x03)) & 0x03)

#define SAP_CONNECT_REQ				0x00
#define SAP_CONNECT_RESP			0x01
#define SAP_DISCONNECT_REQ			0x02
#define SAP_DISCONNECT_RESP			0x03
#define SAP_DISCONNECT_IND			0x04
#define SAP_TRANSFER_APDU_REQ			0x05
#define SAP_TRANSFER_APDU_RESP			0x06
#define SAP_TRANSFER_ATR_REQ			0x07
#define SAP_TRANSFER_ATR_RESP			0x08
#define SAP_POWER_SIM_OFF_REQ			0x09
#define SAP_POWER_SIM_OFF_RESP			0x0A
#define SAP_POWER_SIM_ON_REQ			0x0B
#define SAP_POWER_SIM_ON_RESP			0x0C
#define SAP_RESET_SIM_REQ			0x0D
#define SAP_RESET_SIM_RESP			0x0E
#define SAP_TRANSFER_CARD_READER_STATUS_REQ	0x0F
#define SAP_TRANSFER_CARD_READER_STATUS_RESP	0x10
#define SAP_STATUS_IND				0x11
#define SAP_ERROR_RESP				0x12
#define SAP_SET_TRANSPORT_PROTOCOL_REQ		0x13
#define SAP_SET_TRANSPORT_PROTOCOL_RESP		0x14

#define SAP_PARAM_ID_MAX_MSG_SIZE	0x00
#define SAP_PARAM_ID_CONN_STATUS	0x01
#define SAP_PARAM_ID_RESULT_CODE	0x02
#define SAP_PARAM_ID_DISCONNECT_IND	0x03
#define SAP_PARAM_ID_COMMAND_APDU	0x04
#define SAP_PARAM_ID_COMMAND_APDU7816	0x10
#define SAP_PARAM_ID_RESPONSE_APDU	0x05
#define SAP_PARAM_ID_ATR		0x06
#define SAP_PARAM_ID_CARD_READER_STATUS	0x07
#define SAP_PARAM_ID_STATUS_CHANGE	0x08
#define SAP_PARAM_ID_TRANSPORT_PROTOCOL	0x09

#define SAP_STATUS_OK				0x00
#define SAP_STATUS_CONNECTION_FAILED		0x01
#define SAP_STATUS_MAX_MSG_SIZE_NOT_SUPPORTED	0x02
#define SAP_STATUS_MAX_MSG_SIZE_TOO_SMALL	0x03
#define SAP_STATUS_OK_ONGOING_CALL		0x04

#define SAP_DISCONNECTION_TYPE_GRACEFUL		0x00
#define SAP_DISCONNECTION_TYPE_IMMEDIATE	0x01
#define SAP_DISCONNECTION_TYPE_CLIENT		0xFF

#define SAP_RESULT_OK			0x00
#define SAP_RESULT_ERROR_NO_REASON	0x01
#define SAP_RESULT_ERROR_NOT_ACCESSIBLE	0x02
#define SAP_RESULT_ERROR_POWERED_OFF	0x03
#define SAP_RESULT_ERROR_CARD_REMOVED	0x04
#define SAP_RESULT_ERROR_POWERED_ON	0x05
#define SAP_RESULT_ERROR_NO_DATA	0x06
#define SAP_RESULT_NOT_SUPPORTED	0x07

#define SAP_STATUS_CHANGE_UNKNOWN_ERROR		0x00
#define SAP_STATUS_CHANGE_CARD_RESET		0x01
#define SAP_STATUS_CHANGE_CARD_NOT_ACCESSIBLE	0x02
#define SAP_STATUS_CHANGE_CARD_REMOVED		0x03
#define SAP_STATUS_CHANGE_CARD_INSERTED		0x04
#define SAP_STATUS_CHANGE_CARD_RECOVERED	0x05

#define SAP_TRANSPORT_PROTOCOL_T0	0x00
#define SAP_TRANSPORT_PROTOCOL_T1	0x01

static const char *msg2str(uint8_t msg)
{
	switch (msg) {
	case SAP_CONNECT_REQ:
		return "Connect Req";
	case SAP_CONNECT_RESP:
		return "Connect Resp";
	case SAP_DISCONNECT_REQ:
		return "Disconnect Req";
	case SAP_DISCONNECT_RESP:
		return "Disconnect Resp";
	case SAP_DISCONNECT_IND:
		return "Disconnect Ind";
	case SAP_TRANSFER_APDU_REQ:
		return "Transfer APDU Req";
	case SAP_TRANSFER_APDU_RESP:
		return "Transfer APDU Resp";
	case SAP_TRANSFER_ATR_REQ:
		return "Transfer ATR Req";
	case SAP_TRANSFER_ATR_RESP:
		return "Transfer ATR Resp";
	case SAP_POWER_SIM_OFF_REQ:
		return "Power SIM Off Req";
	case SAP_POWER_SIM_OFF_RESP:
		return "Power SIM Off Resp";
	case SAP_POWER_SIM_ON_REQ:
		return "Power SIM On Req";
	case SAP_POWER_SIM_ON_RESP:
		return "Power SIM On Resp";
	case SAP_RESET_SIM_REQ:
		return "Reset SIM Req";
	case SAP_RESET_SIM_RESP:
		return "Reset SIM Resp";
	case SAP_TRANSFER_CARD_READER_STATUS_REQ:
		return "Transfer Card Reader Status Req";
	case SAP_TRANSFER_CARD_READER_STATUS_RESP:
		return "Transfer Card Reader Status Resp";
	case SAP_STATUS_IND:
		return "Status Ind";
	case SAP_ERROR_RESP:
		return "Error Resp";
	case SAP_SET_TRANSPORT_PROTOCOL_REQ:
		return "Set Transport Protocol Req";
	case SAP_SET_TRANSPORT_PROTOCOL_RESP:
		return "Set Transport Protocol Resp";
	default:
		return "Reserved";
	}
}

static const char *param2str(uint8_t param)
{
	switch (param) {
	case SAP_PARAM_ID_MAX_MSG_SIZE:
		return "MaxMsgSize";
	case SAP_PARAM_ID_CONN_STATUS:
		return "ConnectionStatus";
	case SAP_PARAM_ID_RESULT_CODE:
		return "ResultCode";
	case SAP_PARAM_ID_DISCONNECT_IND:
		return "DisconnectionType";
	case SAP_PARAM_ID_COMMAND_APDU:
		return "CommandAPDU";
	case SAP_PARAM_ID_COMMAND_APDU7816:
		return "CommandAPDU7816";
	case SAP_PARAM_ID_RESPONSE_APDU:
		return "ResponseAPDU";
	case SAP_PARAM_ID_ATR:
		return "ATR";
	case SAP_PARAM_ID_CARD_READER_STATUS:
		return "CardReaderStatus";
	case SAP_PARAM_ID_STATUS_CHANGE:
		return "StatusChange";
	case SAP_PARAM_ID_TRANSPORT_PROTOCOL:
		return "TransportProtocol";
	default:
		return "Reserved";
	}
}

static const char *status2str(uint8_t status)
{
	switch (status) {
	case  SAP_STATUS_OK:
		return "OK, Server can fulfill requirements";
	case  SAP_STATUS_CONNECTION_FAILED:
		return "Error, Server unable to establish connection";
	case  SAP_STATUS_MAX_MSG_SIZE_NOT_SUPPORTED:
		return "Error, Server does not support maximum message size";
	case  SAP_STATUS_MAX_MSG_SIZE_TOO_SMALL:
		return "Error, maximum message size by Client is too small";
	case  SAP_STATUS_OK_ONGOING_CALL:
		return "OK, ongoing call";
	default:
		return "Reserved";
	}
}

static const char *disctype2str(uint8_t disctype)
{
	switch (disctype) {
	case  SAP_DISCONNECTION_TYPE_GRACEFUL:
		return "Graceful";
	case  SAP_DISCONNECTION_TYPE_IMMEDIATE:
		return "Immediate";
	default:
		return "Reserved";
	}
}

static const char *result2str(uint8_t result)
{
	switch (result) {
	case  SAP_RESULT_OK:
		return "OK, request processed correctly";
	case  SAP_RESULT_ERROR_NO_REASON:
		return "Error, no reason defined";
	case SAP_RESULT_ERROR_NOT_ACCESSIBLE:
		return "Error, card not accessible";
	case  SAP_RESULT_ERROR_POWERED_OFF:
		return "Error, card (already) powered off";
	case  SAP_RESULT_ERROR_CARD_REMOVED:
		return "Error, card removed";
	case  SAP_RESULT_ERROR_POWERED_ON:
		return "Error, card already powered on";
	case  SAP_RESULT_ERROR_NO_DATA:
		return "Error, data not available";
	case  SAP_RESULT_NOT_SUPPORTED:
		return "Error, not supported";
	default:
		return "Reserved";
	}
}

static const char *statuschg2str(uint8_t statuschg)
{
	switch (statuschg) {
	case  SAP_STATUS_CHANGE_UNKNOWN_ERROR:
		return "Unknown Error";
	case  SAP_STATUS_CHANGE_CARD_RESET:
		return "Card reset";
	case  SAP_STATUS_CHANGE_CARD_NOT_ACCESSIBLE:
		return "Card not accessible";
	case  SAP_STATUS_CHANGE_CARD_REMOVED:
		return "Card removed";
	case  SAP_STATUS_CHANGE_CARD_INSERTED:
		return "Card inserted";
	case  SAP_STATUS_CHANGE_CARD_RECOVERED:
		return "Card recovered";
	default:
		return "Reserved";
	}
}

static const char *prot2str(uint8_t prot)
{
	switch (prot) {
	case SAP_TRANSPORT_PROTOCOL_T0:
		return "T=0";
	case SAP_TRANSPORT_PROTOCOL_T1:
		return "T=1";
	default:
		return "Reserved";
	}
}

static void parse_parameters(int level, struct frame *frm)
{
	uint8_t param;
	uint16_t len;
	uint8_t pv8;

	while (frm->len > 3) {
		p_indent(level, frm);

		param = p_get_u8(frm);
		p_get_u8(frm);
		len = p_get_u16(frm);

		printf("%s (0x%02x) len %d = ", param2str(param), param, len);

		switch (param) {
		case SAP_PARAM_ID_MAX_MSG_SIZE:
			printf("%d\n", p_get_u16(frm));
			break;
		case SAP_PARAM_ID_CONN_STATUS:
			pv8 = p_get_u8(frm);
			printf("0x%02x (%s)\n", pv8, status2str(pv8));
			break;
		case SAP_PARAM_ID_RESULT_CODE:
		case SAP_PARAM_ID_CARD_READER_STATUS:
			pv8 = p_get_u8(frm);
			printf("0x%02x (%s)\n", pv8, result2str(pv8));
			break;
		case SAP_PARAM_ID_DISCONNECT_IND:
			pv8 = p_get_u8(frm);
			printf("0x%02x (%s)\n", pv8, disctype2str(pv8));
			break;
		case SAP_PARAM_ID_STATUS_CHANGE:
			pv8 = p_get_u8(frm);
			printf("0x%02x (%s)\n", pv8, statuschg2str(pv8));
			break;
		case SAP_PARAM_ID_TRANSPORT_PROTOCOL:
			pv8 = p_get_u8(frm);
			printf("0x%02x (%s)\n", pv8, prot2str(pv8));
			break;
		default:
			printf("\n");
			raw_ndump(level + 1, frm, len);
			frm->ptr += len;
			frm->len -= len;
		}

		/* Skip padding */
		frm->ptr += PADDING4(len);
		frm->len -= PADDING4(len);
	}
}

void sap_dump(int level, struct frame *frm)
{
	uint8_t msg, params;

	msg = p_get_u8(frm);
	params = p_get_u8(frm);

	/* Skip reserved field */
	p_get_u16(frm);

	p_indent(level, frm);

	printf("SAP: %s: params %d\n", msg2str(msg), params);

	parse_parameters(level, frm);
}
