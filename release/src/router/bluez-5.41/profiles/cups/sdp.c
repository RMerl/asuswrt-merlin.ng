/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2003-2010  Marcel Holtmann <marcel@holtmann.org>
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
#include <signal.h>
#include <sys/socket.h>

#include "lib/bluetooth.h"
#include "lib/sdp.h"
#include "lib/sdp_lib.h"

#include "cups.h"

int sdp_search_hcrp(sdp_session_t *sdp, unsigned short *ctrl_psm, unsigned short *data_psm)
{
	sdp_list_t *srch, *attrs, *rsp;
	uuid_t svclass;
	uint16_t attr1, attr2;
	int err;

	if (!sdp)
		return -1;

	sdp_uuid16_create(&svclass, HCR_PRINT_SVCLASS_ID);
	srch = sdp_list_append(NULL, &svclass);

	attr1 = SDP_ATTR_PROTO_DESC_LIST;
	attrs = sdp_list_append(NULL, &attr1);
	attr2 = SDP_ATTR_ADD_PROTO_DESC_LIST;
	attrs = sdp_list_append(attrs, &attr2);

	err = sdp_service_search_attr_req(sdp, srch, SDP_ATTR_REQ_INDIVIDUAL, attrs, &rsp);
	if (err)
		return -1;

	for (; rsp; rsp = rsp->next) {
		sdp_record_t *rec = (sdp_record_t *) rsp->data;
		sdp_list_t *protos;

		if (!sdp_get_access_protos(rec, &protos)) {
			unsigned short psm = sdp_get_proto_port(protos, L2CAP_UUID);
			if (psm > 0) {
				*ctrl_psm = psm;
			}
		}

		if (!sdp_get_add_access_protos(rec, &protos)) {
			unsigned short psm = sdp_get_proto_port(protos, L2CAP_UUID);
			if (psm > 0 && *ctrl_psm > 0) {
				*data_psm = psm;
				return 0;
			}
		}
	}

	return -1;
}

int sdp_search_spp(sdp_session_t *sdp, uint8_t *channel)
{
	sdp_list_t *srch, *attrs, *rsp;
	uuid_t svclass;
	uint16_t attr;
	int err;

	if (!sdp)
		return -1;

	sdp_uuid16_create(&svclass, SERIAL_PORT_SVCLASS_ID);
	srch = sdp_list_append(NULL, &svclass);

	attr = SDP_ATTR_PROTO_DESC_LIST;
	attrs = sdp_list_append(NULL, &attr);

	err = sdp_service_search_attr_req(sdp, srch, SDP_ATTR_REQ_INDIVIDUAL, attrs, &rsp);
	if (err)
		return -1;

	for (; rsp; rsp = rsp->next) {
		sdp_record_t *rec = (sdp_record_t *) rsp->data;
		sdp_list_t *protos;

		if (!sdp_get_access_protos(rec, &protos)) {
			uint8_t ch = sdp_get_proto_port(protos, RFCOMM_UUID);
			if (ch > 0) {
				*channel = ch;
				return 0;
			}
		}
	}

	return -1;
}
