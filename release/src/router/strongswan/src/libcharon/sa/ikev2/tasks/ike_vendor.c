/*
 * Copyright (C) 2009 Martin Willi
 * HSR Hochschule fuer Technik Rapperswil
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
 * Copyright (C) 2016 secunet Security Networks AG
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
	/* length of vendor ID string, 0 for NULL terminated */
	int len;
	/* vendor ID string */
	char *id;
} vid_data_t;

/**
 * Get the data of a vendor ID as a chunk
 */
static chunk_t get_vid_data(vid_data_t *data)
{
	return chunk_create(data->id, data->len ?: strlen(data->id));
}

/**
 * IKEv2 Vendor ID database entry
 */
static vid_data_t vids[] = {
	/* strongSwan MD5("strongSwan") */
	{ "strongSwan", EXT_STRONGSWAN, "send_vendor_id", 16,
	  "\x88\x2f\xe5\x6d\x6f\xd2\x0d\xbc\x22\x51\x61\x3b\x2e\xbe\x5b\xeb"},
	{ "Cisco Delete Reason", 0, NULL, 0,
	  "CISCO-DELETE-REASON" },
	{ "Cisco FlexVPN Supported", 0, NULL, 0,
	  "FLEXVPN-SUPPORTED" },
	{ "Cisco Copyright (c) 2009", 0, NULL, 0,
	  "CISCO(COPYRIGHT)&Copyright (c) 2009 Cisco Systems, Inc." },
	{ "FRAGMENTATION", 0, NULL, 16,
	  "\x40\x48\xb7\xd5\x6e\xbc\xe8\x85\x25\xe7\xde\x7f\x00\xd6\xc2\xd3"},
	{ "MS NT5 ISAKMPOAKLEY v7", 0, NULL, 20,
	  "\x1e\x2b\x51\x69\x05\x99\x1c\x7d\x7c\x96\xfc\xbf\xb5\x87\xe4\x61\x00\x00\x00\x07"},
	{ "MS NT5 ISAKMPOAKLEY v8", 0, NULL, 20,
	  "\x1e\x2b\x51\x69\x05\x99\x1c\x7d\x7c\x96\xfc\xbf\xb5\x87\xe4\x61\x00\x00\x00\x08"},
	{ "MS NT5 ISAKMPOAKLEY v9", 0, NULL, 20,
	  "\x1e\x2b\x51\x69\x05\x99\x1c\x7d\x7c\x96\xfc\xbf\xb5\x87\xe4\x61\x00\x00\x00\x09"},
	{ "MS-Negotiation Discovery Capable", 0, NULL, 16,
	  "\xfb\x1d\xe3\xcd\xf3\x41\xb7\xea\x16\xb7\xe5\xbe\x08\x55\xf1\x20"},
	{ "Vid-Initial-Contact", 0, NULL, 16,
	  "\x26\x24\x4d\x38\xed\xdb\x61\xb3\x17\x2a\x36\xe3\xd0\xcf\xb8\x19"},
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
				if (chunk_equals(get_vid_data(&vids[i]), data))
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
