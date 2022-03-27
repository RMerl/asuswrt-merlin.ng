/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2002-2003  Maxim Krasnyansky <maxk@qualcomm.com>
 *  Copyright (C) 2002-2010  Marcel Holtmann <marcel@holtmann.org>
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
#include <sys/socket.h>

#include "lib/bluetooth.h"
#include "lib/hci.h"
#include "lib/hci_lib.h"

int main(int argc, char *argv[])
{
	uint32_t type_mask;
	uint32_t event_mask[2];
	uint32_t ocf_mask[4];

	/* Packet types */
	memset(&type_mask, 0, sizeof(type_mask));
	hci_set_bit(HCI_EVENT_PKT, &type_mask);

	printf("Type mask:        { 0x%02x }\n", type_mask);

	/* Events */
	memset(event_mask, 0, sizeof(event_mask));
	hci_set_bit(EVT_INQUIRY_COMPLETE,			event_mask);
	hci_set_bit(EVT_INQUIRY_RESULT,				event_mask);
	hci_set_bit(EVT_CONN_COMPLETE,				event_mask);
	hci_set_bit(EVT_CONN_REQUEST,				event_mask);
	hci_set_bit(EVT_DISCONN_COMPLETE,			event_mask);
	hci_set_bit(EVT_AUTH_COMPLETE,				event_mask);
	hci_set_bit(EVT_REMOTE_NAME_REQ_COMPLETE,		event_mask);
	hci_set_bit(EVT_ENCRYPT_CHANGE,				event_mask);
	hci_set_bit(EVT_READ_REMOTE_FEATURES_COMPLETE,		event_mask);
	hci_set_bit(EVT_READ_REMOTE_VERSION_COMPLETE,		event_mask);
	hci_set_bit(EVT_CMD_COMPLETE,				event_mask);
	hci_set_bit(EVT_CMD_STATUS,				event_mask);
	hci_set_bit(EVT_READ_CLOCK_OFFSET_COMPLETE,		event_mask);
	hci_set_bit(EVT_INQUIRY_RESULT_WITH_RSSI,		event_mask);
	hci_set_bit(EVT_READ_REMOTE_EXT_FEATURES_COMPLETE,	event_mask);
	hci_set_bit(EVT_SYNC_CONN_COMPLETE,			event_mask);
	hci_set_bit(EVT_SYNC_CONN_CHANGED,			event_mask);
	hci_set_bit(EVT_EXTENDED_INQUIRY_RESULT,		event_mask);

	printf("Event mask:       { 0x%08x, 0x%08x }\n",
					event_mask[0], event_mask[1]);

	/* OGF_LINK_CTL */
	memset(ocf_mask, 0, sizeof(ocf_mask));
	hci_set_bit(OCF_INQUIRY,			ocf_mask);
	hci_set_bit(OCF_INQUIRY_CANCEL,			ocf_mask);
	hci_set_bit(OCF_REMOTE_NAME_REQ,		ocf_mask);
	hci_set_bit(OCF_REMOTE_NAME_REQ_CANCEL,		ocf_mask);
	hci_set_bit(OCF_READ_REMOTE_FEATURES,		ocf_mask);
	hci_set_bit(OCF_READ_REMOTE_EXT_FEATURES,	ocf_mask);
	hci_set_bit(OCF_READ_REMOTE_VERSION,		ocf_mask);
	hci_set_bit(OCF_READ_CLOCK_OFFSET,		ocf_mask);
	hci_set_bit(OCF_READ_LMP_HANDLE,		ocf_mask);

	printf("OGF_LINK_CTL:     { 0x%08x, 0x%08x, 0x%08x, 0x%02x }\n",
			ocf_mask[0], ocf_mask[1], ocf_mask[2], ocf_mask[3]);

	/* OGF_LINK_POLICY */
	memset(ocf_mask, 0, sizeof(ocf_mask));
	hci_set_bit(OCF_ROLE_DISCOVERY,			ocf_mask);
	hci_set_bit(OCF_READ_LINK_POLICY,		ocf_mask);
	hci_set_bit(OCF_READ_DEFAULT_LINK_POLICY,	ocf_mask);

	printf("OGF_LINK_POLICY:  { 0x%08x, 0x%08x, 0x%08x, 0x%02x }\n",
			ocf_mask[0], ocf_mask[1], ocf_mask[2], ocf_mask[3]);

	/* OGF_HOST_CTL */
	memset(ocf_mask, 0, sizeof(ocf_mask));
	hci_set_bit(OCF_READ_PIN_TYPE,			ocf_mask);
	hci_set_bit(OCF_READ_LOCAL_NAME,		ocf_mask);
	hci_set_bit(OCF_READ_CONN_ACCEPT_TIMEOUT,	ocf_mask);
	hci_set_bit(OCF_READ_PAGE_TIMEOUT,		ocf_mask);
	hci_set_bit(OCF_READ_SCAN_ENABLE,		ocf_mask);
	hci_set_bit(OCF_READ_PAGE_ACTIVITY,		ocf_mask);
	hci_set_bit(OCF_READ_INQ_ACTIVITY,		ocf_mask);
	hci_set_bit(OCF_READ_AUTH_ENABLE,		ocf_mask);
	hci_set_bit(OCF_READ_ENCRYPT_MODE,		ocf_mask);
	hci_set_bit(OCF_READ_CLASS_OF_DEV,		ocf_mask);
	hci_set_bit(OCF_READ_VOICE_SETTING,		ocf_mask);
	hci_set_bit(OCF_READ_AUTOMATIC_FLUSH_TIMEOUT,	ocf_mask);
	hci_set_bit(OCF_READ_NUM_BROADCAST_RETRANS,	ocf_mask);
	hci_set_bit(OCF_READ_HOLD_MODE_ACTIVITY,	ocf_mask);
	hci_set_bit(OCF_READ_TRANSMIT_POWER_LEVEL,	ocf_mask);
	hci_set_bit(OCF_READ_LINK_SUPERVISION_TIMEOUT,	ocf_mask);
	hci_set_bit(OCF_READ_NUM_SUPPORTED_IAC,		ocf_mask);
	hci_set_bit(OCF_READ_CURRENT_IAC_LAP,		ocf_mask);
	hci_set_bit(OCF_READ_PAGE_SCAN_PERIOD_MODE,	ocf_mask);
	hci_set_bit(OCF_READ_PAGE_SCAN_MODE,		ocf_mask);
	hci_set_bit(OCF_READ_INQUIRY_SCAN_TYPE,		ocf_mask);
	hci_set_bit(OCF_READ_INQUIRY_MODE,		ocf_mask);
	hci_set_bit(OCF_READ_PAGE_SCAN_TYPE,		ocf_mask);
	hci_set_bit(OCF_READ_AFH_MODE,			ocf_mask);
	hci_set_bit(OCF_READ_EXT_INQUIRY_RESPONSE,	ocf_mask);
	hci_set_bit(OCF_READ_SIMPLE_PAIRING_MODE,	ocf_mask);
	hci_set_bit(OCF_READ_INQ_RESPONSE_TX_POWER_LEVEL,	ocf_mask);
	hci_set_bit(OCF_READ_DEFAULT_ERROR_DATA_REPORTING,	ocf_mask);

	printf("OGF_HOST_CTL:     { 0x%08x, 0x%08x, 0x%08x, 0x%02x }\n",
			ocf_mask[0], ocf_mask[1], ocf_mask[2], ocf_mask[3]);

	/* OGF_INFO_PARAM */
	memset(ocf_mask, 0, sizeof(ocf_mask));
	hci_set_bit(OCF_READ_LOCAL_VERSION,		ocf_mask);
	hci_set_bit(OCF_READ_LOCAL_COMMANDS,		ocf_mask);
	hci_set_bit(OCF_READ_LOCAL_FEATURES,		ocf_mask);
	hci_set_bit(OCF_READ_LOCAL_EXT_FEATURES,	ocf_mask);
	hci_set_bit(OCF_READ_BUFFER_SIZE,		ocf_mask);
	hci_set_bit(OCF_READ_COUNTRY_CODE,		ocf_mask);
	hci_set_bit(OCF_READ_BD_ADDR,			ocf_mask);

	printf("OGF_INFO_PARAM:   { 0x%08x, 0x%08x, 0x%08x, 0x%02x }\n",
			ocf_mask[0], ocf_mask[1], ocf_mask[2], ocf_mask[3]);

	/* OGF_STATUS_PARAM */
	memset(ocf_mask, 0, sizeof(ocf_mask));
	hci_set_bit(OCF_READ_FAILED_CONTACT_COUNTER,	ocf_mask);
	hci_set_bit(OCF_READ_LINK_QUALITY,		ocf_mask);
	hci_set_bit(OCF_READ_RSSI,			ocf_mask);
	hci_set_bit(OCF_READ_AFH_MAP,			ocf_mask);
	hci_set_bit(OCF_READ_CLOCK,			ocf_mask);

	printf("OGF_STATUS_PARAM: { 0x%08x, 0x%08x, 0x%08x, 0x%02x }\n",
			ocf_mask[0], ocf_mask[1], ocf_mask[2], ocf_mask[3]);

	return 0;
}
