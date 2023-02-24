/*
 * Copyright (C) 2009 Martin Willi
 *
 * Copyright (C) secunet Security Networks AG
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.  See <http://www.fsf.org/copyleft/gpl.txt>.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 */

/*
 * Copyright (C) 2016 Thomas Egerer
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include "ike_vendor.h"

#include <daemon.h>
#include <encoding/payloads/vendor_id_payload.h>

typedef struct private_ike_vendor_t private_ike_vendor_t;

/**
 * Private data of an ike_vendor_t object.
 */
struct private_ike_vendor_t {

	/**
	 * Public ike_vendor_t interface.
	 */
	ike_vendor_t public;

	/**
	 * Associated IKE_SA
	 */
	ike_sa_t *ike_sa;

	/**
	 * Are we the initiator of this task
	 */
	bool initiator;
};

/**
 * Vendor ID database entry
 */
typedef struct {
	/* Description */
	char *desc;
	/* extension flag negotiated with vendor ID, if any */
	ike_extension_t extension;
	/* Value from strongswan.conf, whether to send vendor ID */
	char *setting;
	/* Prefix matching only */
	bool prefix;
	/* length of vendor ID string, 0 for NULL terminated */
	int len;
	/* vendor ID string */
	char *id;
} vid_data_t;

/**
 * Get the data of a vendor ID as a chunk
 */
static inline chunk_t get_vid_data(vid_data_t *data)
{
	return chunk_create(data->id, data->len ?: strlen(data->id));
}

/**
 * IKEv2 Vendor ID database entry
 */
static vid_data_t vids[] = {
	/* strongSwan MD5("strongSwan") */
	{ "strongSwan", EXT_STRONGSWAN, "send_vendor_id", FALSE, 16,
	  "\x88\x2f\xe5\x6d\x6f\xd2\x0d\xbc\x22\x51\x61\x3b\x2e\xbe\x5b\xeb"},
	{ "Cisco Delete Reason", 0, NULL, FALSE, 0,
	  "CISCO-DELETE-REASON" },
	{ "Cisco FlexVPN Supported", 0, "cisco_flexvpn", FALSE, 0,
	  "FLEXVPN-SUPPORTED" },
	{ "Cisco Copyright (c) 2009", 0, NULL, FALSE, 0,
	  "CISCO(COPYRIGHT)&Copyright (c) 2009 Cisco Systems, Inc." },
	{ "FRAGMENTATION", 0, NULL, FALSE, 16,
	  "\x40\x48\xb7\xd5\x6e\xbc\xe8\x85\x25\xe7\xde\x7f\x00\xd6\xc2\xd3"},
	{ "MS NT5 ISAKMPOAKLEY v7", 0, NULL, FALSE, 20,
	  "\x1e\x2b\x51\x69\x05\x99\x1c\x7d\x7c\x96\xfc\xbf\xb5\x87\xe4\x61\x00\x00\x00\x07"},
	{ "MS NT5 ISAKMPOAKLEY v8", 0, NULL, FALSE, 20,
	  "\x1e\x2b\x51\x69\x05\x99\x1c\x7d\x7c\x96\xfc\xbf\xb5\x87\xe4\x61\x00\x00\x00\x08"},
	{ "MS NT5 ISAKMPOAKLEY v9", 0, NULL, FALSE, 20,
	  "\x1e\x2b\x51\x69\x05\x99\x1c\x7d\x7c\x96\xfc\xbf\xb5\x87\xe4\x61\x00\x00\x00\x09"},
	{ "MS-Negotiation Discovery Capable", 0, NULL, FALSE, 16,
	  "\xfb\x1d\xe3\xcd\xf3\x41\xb7\xea\x16\xb7\xe5\xbe\x08\x55\xf1\x20"},
	{ "Vid-Initial-Contact", 0, NULL, FALSE, 16,
	  "\x26\x24\x4d\x38\xed\xdb\x61\xb3\x17\x2a\x36\xe3\xd0\xcf\xb8\x19"},
	/* Truncated MD5("ALTIGA GATEWAY") plus two version bytes */
	{ "Cisco VPN Concentrator", 0, NULL, TRUE, 14,
	  "\x1f\x07\xf7\x0e\xaa\x65\x14\xd3\xb0\xfa\x96\x54\x2a\x50\x00\x00"},
	/* MD5("ALTIGA NETWORKS") */
	{ "Cisco VPN 3000 client", 0, NULL, FALSE, 16,
	  "\xf6\xf7\xef\xc7\xf5\xae\xb8\xcb\x15\x8c\xb9\xd0\x94\xba\x69\xe7"},
	{ "ZyXEL ZyWALL Router", 0, NULL, FALSE, 20,
	  "\xb8\x58\xd1\xad\xdd\x08\xc1\xe8\xad\xaf\xea\x15\x06\x08\xaa\x44\x97\xaa\x6c\xc8"},
	{ "ZyXEL ZyWALL USG 100", 0, NULL, FALSE, 14,
	  "\xf7\x58\xf2\x26\x68\x75\x0f\x03\xb0\x8d\xf6\xeb\xe1\xd0"},
	{ "ZyXEL ZyWALL", 0, NULL, FALSE, 20,
	  "\x62\x50\x27\x74\x9d\x5a\xb9\x7f\x56\x16\xc1\x60\x27\x65\xcf\x48\x0a\x3b\x7d\x0b"},
	{ "Sonicwall 7", 0, NULL, TRUE, 8,
	  "\x5b\x36\x2b\xc8\x20\xf6\x00\x07"},
	{ "Sonicwall 8", 0, NULL, TRUE, 8,
	  "\x5b\x36\x2b\xc8\x20\xf6\x00\x08"},
	{ "Sonicwall a", 0, NULL, TRUE, 8,
	  "\x40\x4b\xf4\x39\x52\x2c\xa3\xf6"},
	{ "Sonicwall b", 0, NULL, TRUE, 8,
	  "\xda\x8e\x93\x78\x80\x01\x00\x00"},
	{ "Sonicwall c", 0, NULL, TRUE, 8,
	  "\x5b\x36\x2b\xc8\x20\xf7\x00\x01"},
	{ "Fortigate", 0, NULL, FALSE, 16,
	  "\x1d\x6e\x17\x8f\x6c\x2c\x0b\xe2\x84\x98\x54\x65\x45\x0f\xe9\xd4"},
	{ "Checkpoint Firewall", 0, NULL, TRUE, 20,
	  "\xf4\xed\x19\xe0\xc1\x14\xeb\x51\x6f\xaa\xac\x0e\xe3\x7d\xaf\x28\x07\xb4\x38\x1f"},
	{ "NetScreen Technologies", 0, NULL, TRUE, 20,
	  "\x69\x93\x69\x22\x87\x41\xc6\xd4\xca\x09\x4c\x93\xe2\x42\xc9\xde\x19\xe7\xb7\xc6"},
	{ "Juniper SRX", 0, NULL, FALSE, 20,
	  "\xfd\x80\x88\x04\xdf\x73\xb1\x51\x50\x70\x9d\x87\x80\x44\xcd\xe0\xac\x1e\xfc\xde"},
};

METHOD(task_t, build, status_t,
	private_ike_vendor_t *this, message_t *message)
{
	vendor_id_payload_t *vid;
	bool send_vid;
	int i;

	for (i = 0; i < countof(vids); i++)
	{
		send_vid = FALSE;

		if (vids[i].setting)
		{
			send_vid = lib->settings->get_bool(lib->settings, "%s.%s", send_vid,
											   lib->ns, vids[i].setting);
		}
		if (send_vid)
		{
			DBG2(DBG_IKE, "sending %s vendor ID", vids[i].desc);
			vid = vendor_id_payload_create_data(PLV2_VENDOR_ID,
										chunk_clone(get_vid_data(&vids[i])));
			message->add_payload(message, &vid->payload_interface);
		}
	}

	return this->initiator ? NEED_MORE : SUCCESS;
}

/**
 * Check if the given known vendor ID matches a received VID or its prefix
 */
static inline bool known_vid(vid_data_t *vid, chunk_t data)
{
	chunk_t known = get_vid_data(vid);

	if (vid->prefix)
	{
		data.len = min(data.len, known.len);
	}
	return chunk_equals(known, data);
}

METHOD(task_t, process, status_t,
	private_ike_vendor_t *this, message_t *message)
{
	enumerator_t *enumerator;
	payload_t *payload;
	int i;

	enumerator = message->create_payload_enumerator(message);
	while (enumerator->enumerate(enumerator, &payload))
	{
		if (payload->get_type(payload) == PLV2_VENDOR_ID)
		{
			vendor_id_payload_t *vid;
			chunk_t data;
			bool found = FALSE;

			vid = (vendor_id_payload_t*)payload;
			data = vid->get_data(vid);

			for (i = 0; i < countof(vids); i++)
			{
				if (known_vid(&vids[i], data))
				{
					DBG1(DBG_IKE, "received %s vendor ID", vids[i].desc);
					if (vids[i].extension)
					{
						this->ike_sa->enable_extension(this->ike_sa,
													   vids[i].extension);
					}
					found = TRUE;
					break;
				}
			}
			if (!found)
			{
				DBG1(DBG_ENC, "received unknown vendor ID: %#B", &data);
			}
		}
	}
	enumerator->destroy(enumerator);

	return this->initiator ? SUCCESS : NEED_MORE;
}

METHOD(task_t, migrate, void,
	private_ike_vendor_t *this, ike_sa_t *ike_sa)
{
	this->ike_sa = ike_sa;
}

METHOD(task_t, get_type, task_type_t,
	private_ike_vendor_t *this)
{
	return TASK_IKE_VENDOR;
}

METHOD(task_t, destroy, void,
	private_ike_vendor_t *this)
{
	free(this);
}

/**
 * See header
 */
ike_vendor_t *ike_vendor_create(ike_sa_t *ike_sa, bool initiator)
{
	private_ike_vendor_t *this;

	INIT(this,
		.public = {
			.task = {
				.build = _build,
				.process = _process,
				.migrate = _migrate,
				.get_type = _get_type,
				.destroy = _destroy,
			},
		},
		.initiator = initiator,
		.ike_sa = ike_sa,
	);

	return &this->public;
}
