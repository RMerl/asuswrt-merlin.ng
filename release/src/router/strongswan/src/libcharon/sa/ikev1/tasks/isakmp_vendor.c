/*
 * Copyright (C) 2012-2013 Tobias Brunner
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
 * Copyright (C) 2012-2014 Volker Rümelin
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

#include "isakmp_vendor.h"

#include <daemon.h>
#include <encoding/payloads/vendor_id_payload.h>

typedef struct private_isakmp_vendor_t private_isakmp_vendor_t;

/**
 * Private data of an isakmp_vendor_t object.
 */
struct private_isakmp_vendor_t {

	/**
	 * Public isakmp_vendor_t interface.
	 */
	isakmp_vendor_t public;

	/**
	 * Associated IKE_SA
	 */
	ike_sa_t *ike_sa;

	/**
	 * Are we the initiator of this task
	 */
	bool initiator;

	/**
	 * Index of best nat traversal VID found
	 */
	int best_natt_ext;

	/**
	 * Number of times we have been invoked
	 */
	int count;
};

/**
 * IKEv1 Vendor ID database
 */
static struct {
	/* Description */
	char *desc;
	/* extension flag negotiated with vendor ID, if any */
	ike_extension_t extension;
	/* send yourself? */
	bool send;
	/* stored id is just a prefix for a longer, more specific one */
	bool prefix;
	/* length of vendor ID string */
	int len;
	/* vendor ID string */
	char *id;
} vendor_ids[] = {

	/* strongSwan MD5("strongSwan") */
	{ "strongSwan", EXT_STRONGSWAN, FALSE, FALSE, 16,
	  "\x88\x2f\xe5\x6d\x6f\xd2\x0d\xbc\x22\x51\x61\x3b\x2e\xbe\x5b\xeb"},

	/* XAuth, MD5("draft-ietf-ipsra-isakmp-xauth-06.txt") */
	{ "XAuth", EXT_XAUTH, TRUE, FALSE, 8,
	  "\x09\x00\x26\x89\xdf\xd6\xb7\x12"},

	/* Dead peer detection, RFC 3706 */
	{ "DPD", EXT_DPD, TRUE, FALSE, 16,
	  "\xaf\xca\xd7\x13\x68\xa1\xf1\xc9\x6b\x86\x96\xfc\x77\x57\x01\x00"},

	/* CISCO-UNITY, similar to DPD the last two bytes indicate the version */
	{ "Cisco Unity", EXT_CISCO_UNITY, FALSE, FALSE, 16,
	  "\x12\xf5\xf2\x8c\x45\x71\x68\xa9\x70\x2d\x9f\xe2\x74\xcc\x01\x00"},

	/* Proprietary IKE fragmentation extension. Capabilities are handled
	 * specially on receipt of this VID. Windows peers send this VID
	 * without capabilities, but accept it with and without capabilities. */
	{ "FRAGMENTATION", EXT_IKE_FRAGMENTATION, FALSE, FALSE, 20,
	  "\x40\x48\xb7\xd5\x6e\xbc\xe8\x85\x25\xe7\xde\x7f\x00\xd6\xc2\xd3\x80\x00\x00\x00"},

	/* Windows peers send this VID and a version number */
	{ "MS NT5 ISAKMPOAKLEY", EXT_MS_WINDOWS, FALSE, TRUE, 16,
	  "\x1e\x2b\x51\x69\x05\x99\x1c\x7d\x7c\x96\xfc\xbf\xb5\x87\xe4\x61\x00\x00\x00\x00"},

	/* Truncated MD5("ALTIGA GATEWAY") plus two version bytes */
	{ "Cisco VPN Concentrator", 0, FALSE, TRUE, 14,
	  "\x1f\x07\xf7\x0e\xaa\x65\x14\xd3\xb0\xfa\x96\x54\x2a\x50\x00\x00"},

	/* MD5("ALTIGA NETWORKS") */
	{ "Cisco VPN 3000 client", 0, FALSE, FALSE, 16,
	  "\xf6\xf7\xef\xc7\xf5\xae\xb8\xcb\x15\x8c\xb9\xd0\x94\xba\x69\xe7"},

	{ "KAME/racoon", 0, FALSE, FALSE, 16,
	  "\x70\x03\xcb\xc1\x09\x7d\xbe\x9c\x26\x00\xba\x69\x83\xbc\x8b\x35"},

	{ "ZyXEL ZyWALL Router", 0, FALSE, FALSE, 20,
	  "\xb8\x58\xd1\xad\xdd\x08\xc1\xe8\xad\xaf\xea\x15\x06\x08\xaa\x44\x97\xaa\x6c\xc8"},

	{ "ZyXEL ZyWALL USG 100", 0, FALSE, FALSE, 14,
	  "\xf7\x58\xf2\x26\x68\x75\x0f\x03\xb0\x8d\xf6\xeb\xe1\xd0"},

	{ "ZyXEL ZyWALL", 0, FALSE, FALSE, 20,
	  "\x62\x50\x27\x74\x9d\x5a\xb9\x7f\x56\x16\xc1\x60\x27\x65\xcf\x48\x0a\x3b\x7d\x0b"},

	{ "Sonicwall 1", 0, FALSE, TRUE, 8,
	  "\x5b\x36\x2b\xc8\x20\xf6\x00\x01"},

	{ "Sonicwall 2", 0, FALSE, TRUE, 8,
	  "\x5b\x36\x2b\xc8\x20\xf6\x00\x02"},

	{ "Sonicwall 3", 0, FALSE, TRUE, 8,
	  "\x5b\x36\x2b\xc8\x20\xf6\x00\x03"},

	{ "Sonicwall 5", 0, FALSE, TRUE, 8,
	  "\x5b\x36\x2b\xc8\x20\xf6\x00\x05"},

	{ "Sonicwall 6", 0, FALSE, TRUE, 8,
	  "\x5b\x36\x2b\xc8\x20\xf6\x00\x06"},

	{ "Sonicwall 7", 0, FALSE, TRUE, 8,
	  "\x5b\x36\x2b\xc8\x20\xf6\x00\x07"},

	{ "Sonicwall 8", 0, FALSE, TRUE, 8,
	  "\x5b\x36\x2b\xc8\x20\xf6\x00\x08"},

	{ "Sonicwall a", 0, FALSE, TRUE, 8,
	  "\x40\x4b\xf4\x39\x52\x2c\xa3\xf6"},

	{ "Sonicwall b", 0, FALSE, TRUE, 8,
	  "\xda\x8e\x93\x78\x80\x01\x00\x00"},

	{ "Sonicwall c", 0, FALSE, TRUE, 8,
	  "\x5b\x36\x2b\xc8\x20\xf7\x00\x01"},

	{ "Fortigate", 0, FALSE, FALSE, 16,
	  "\x1d\x6e\x17\x8f\x6c\x2c\x0b\xe2\x84\x98\x54\x65\x45\x0f\xe9\xd4"},

	/* Checkpoint devices send a version blob after this VID */
	{ "Checkpoint Firewall", 0, FALSE, TRUE, 20,
	  "\xf4\xed\x19\xe0\xc1\x14\xeb\x51\x6f\xaa\xac\x0e\xe3\x7d\xaf\x28\x07\xb4\x38\x1f"},

	/* Juniper SRX and Netscreen devices send this VID and a version number */
	{ "NetScreen Technologies", 0, FALSE, TRUE, 20,
	  "\x69\x93\x69\x22\x87\x41\xc6\xd4\xca\x09\x4c\x93\xe2\x42\xc9\xde\x19\xe7\xb7\xc6"},

	/* Probably the Juniper SRX VID */
	{ "Juniper SRX", 0, FALSE, FALSE, 20,
	  "\xfd\x80\x88\x04\xdf\x73\xb1\x51\x50\x70\x9d\x87\x80\x44\xcd\xe0\xac\x1e\xfc\xde"},

}, vendor_natt_ids[] = {

	/* NAT-Traversal VIDs ordered by preference */

	/* NAT-Traversal, MD5("RFC 3947") */
	{ "NAT-T (RFC 3947)", EXT_NATT, TRUE, FALSE, 16,
	  "\x4a\x13\x1c\x81\x07\x03\x58\x45\x5c\x57\x28\xf2\x0e\x95\x45\x2f"},

	{ "draft-ietf-ipsec-nat-t-ike-03", EXT_NATT | EXT_NATT_DRAFT_02_03,
	  FALSE, FALSE, 16,
	  "\x7d\x94\x19\xa6\x53\x10\xca\x6f\x2c\x17\x9d\x92\x15\x52\x9d\x56"},

	{ "draft-ietf-ipsec-nat-t-ike-02", EXT_NATT | EXT_NATT_DRAFT_02_03,
	  FALSE, FALSE, 16,
	  "\xcd\x60\x46\x43\x35\xdf\x21\xf8\x7c\xfd\xb2\xfc\x68\xb6\xa4\x48"},

	{ "draft-ietf-ipsec-nat-t-ike-02\\n", EXT_NATT | EXT_NATT_DRAFT_02_03,
	  TRUE, FALSE, 16,
	  "\x90\xcb\x80\x91\x3e\xbb\x69\x6e\x08\x63\x81\xb5\xec\x42\x7b\x1f"},

	{ "draft-ietf-ipsec-nat-t-ike-08", 0, FALSE, FALSE, 16,
	  "\x8f\x8d\x83\x82\x6d\x24\x6b\x6f\xc7\xa8\xa6\xa4\x28\xc1\x1d\xe8"},

	{ "draft-ietf-ipsec-nat-t-ike-07", 0, FALSE, FALSE, 16,
	  "\x43\x9b\x59\xf8\xba\x67\x6c\x4c\x77\x37\xae\x22\xea\xb8\xf5\x82"},

	{ "draft-ietf-ipsec-nat-t-ike-06", 0, FALSE, FALSE, 16,
	  "\x4d\x1e\x0e\x13\x6d\xea\xfa\x34\xc4\xf3\xea\x9f\x02\xec\x72\x85"},

	{ "draft-ietf-ipsec-nat-t-ike-05", 0, FALSE, FALSE, 16,
	  "\x80\xd0\xbb\x3d\xef\x54\x56\x5e\xe8\x46\x45\xd4\xc8\x5c\xe3\xee"},

	{ "draft-ietf-ipsec-nat-t-ike-04", 0, FALSE, FALSE, 16,
	  "\x99\x09\xb6\x4e\xed\x93\x7c\x65\x73\xde\x52\xac\xe9\x52\xfa\x6b"},

	{ "draft-ietf-ipsec-nat-t-ike-00", 0, FALSE, FALSE, 16,
	  "\x44\x85\x15\x2d\x18\xb6\xbb\xcd\x0b\xe8\xa8\x46\x95\x79\xdd\xcc"},

	{ "draft-ietf-ipsec-nat-t-ike", 0, FALSE, FALSE, 16,
	  "\x4d\xf3\x79\x28\xe9\xfc\x4f\xd1\xb3\x26\x21\x70\xd5\x15\xc6\x62"},

	{ "draft-stenberg-ipsec-nat-traversal-02", 0, FALSE, FALSE, 16,
	  "\x61\x05\xc4\x22\xe7\x68\x47\xe4\x3f\x96\x84\x80\x12\x92\xae\xcd"},

	{ "draft-stenberg-ipsec-nat-traversal-01", 0, FALSE, FALSE, 16,
	  "\x27\xba\xb5\xdc\x01\xea\x07\x60\xea\x4e\x31\x90\xac\x27\xc0\xd0"},

};

/**
 * According to racoon 0x80000000 seems to indicate support for fragmentation
 * of Aggressive and Main mode messages.  0x40000000 seems to indicate support
 * for fragmentation of base ISAKMP messages (Cisco adds that and thus sends
 * 0xc0000000)
 */
static const uint32_t fragmentation_ike = 0x80000000;

static bool is_known_vid(chunk_t data, int i)
{
	switch (vendor_ids[i].extension)
	{
		case EXT_IKE_FRAGMENTATION:
			if (data.len >= 16 && memeq(data.ptr, vendor_ids[i].id, 16))
			{
				switch (data.len)
				{
					case 16:
						return TRUE;
					case 20:
						return untoh32(&data.ptr[16]) & fragmentation_ike;
				}
			}
			return FALSE;
		case EXT_CISCO_UNITY:
			return data.len == 16 && memeq(data.ptr, vendor_ids[i].id, 14);
		default:
			break;
	}
	if (vendor_ids[i].prefix)
	{
		data.len = min(data.len, vendor_ids[i].len);
	}
	return chunk_equals(data, chunk_create(vendor_ids[i].id,
										   vendor_ids[i].len));
}

/**
 * Add supported vendor ID payloads
 */
static void build(private_isakmp_vendor_t *this, message_t *message)
{
	vendor_id_payload_t *vid_payload;
	bool strongswan, cisco_unity, fragmentation;
	ike_cfg_t *ike_cfg;
	int i;

	strongswan = lib->settings->get_bool(lib->settings,
										 "%s.send_vendor_id", FALSE, lib->ns);
	cisco_unity = lib->settings->get_bool(lib->settings,
										 "%s.cisco_unity", FALSE, lib->ns);
	ike_cfg = this->ike_sa->get_ike_cfg(this->ike_sa);
	fragmentation = ike_cfg->fragmentation(ike_cfg) != FRAGMENTATION_NO;
	if (!this->initiator && fragmentation)
	{
		fragmentation = this->ike_sa->supports_extension(this->ike_sa,
														 EXT_IKE_FRAGMENTATION);
	}
	for (i = 0; i < countof(vendor_ids); i++)
	{
		if (vendor_ids[i].send ||
		   (vendor_ids[i].extension == EXT_STRONGSWAN && strongswan) ||
		   (vendor_ids[i].extension == EXT_CISCO_UNITY && cisco_unity) ||
		   (vendor_ids[i].extension == EXT_IKE_FRAGMENTATION && fragmentation))
		{
			DBG2(DBG_IKE, "sending %s vendor ID", vendor_ids[i].desc);
			vid_payload = vendor_id_payload_create_data(PLV1_VENDOR_ID,
				chunk_clone(chunk_create(vendor_ids[i].id, vendor_ids[i].len)));
			message->add_payload(message, &vid_payload->payload_interface);
		}
	}
	for (i = 0; i < countof(vendor_natt_ids); i++)
	{
		if ((this->initiator && vendor_natt_ids[i].send) ||
			this->best_natt_ext == i)
		{
			DBG2(DBG_IKE, "sending %s vendor ID", vendor_natt_ids[i].desc);
			vid_payload = vendor_id_payload_create_data(PLV1_VENDOR_ID,
							chunk_clone(chunk_create(vendor_natt_ids[i].id,
													 vendor_natt_ids[i].len)));
			message->add_payload(message, &vid_payload->payload_interface);
		}
	}
}

/**
 * Process vendor ID payloads
 */
static void process(private_isakmp_vendor_t *this, message_t *message)
{
	enumerator_t *enumerator;
	payload_t *payload;
	int i;

	enumerator = message->create_payload_enumerator(message);
	while (enumerator->enumerate(enumerator, &payload))
	{
		if (payload->get_type(payload) == PLV1_VENDOR_ID)
		{
			vendor_id_payload_t *vid;
			bool found = FALSE;
			chunk_t data;

			vid = (vendor_id_payload_t*)payload;
			data = vid->get_data(vid);

			for (i = 0; i < countof(vendor_ids); i++)
			{
				if (is_known_vid(data, i))
				{
					DBG1(DBG_IKE, "received %s vendor ID", vendor_ids[i].desc);
					if (vendor_ids[i].extension)
					{
						this->ike_sa->enable_extension(this->ike_sa,
													   vendor_ids[i].extension);
					}
					found = TRUE;
					break;
				}
			}
			if (!found)
			{
				for (i = 0; i < countof(vendor_natt_ids); i++)
				{
					if (chunk_equals(data, chunk_create(vendor_natt_ids[i].id,
													vendor_natt_ids[i].len)))
					{
						DBG1(DBG_IKE, "received %s vendor ID",
							 vendor_natt_ids[i].desc);
						if (vendor_natt_ids[i].extension &&
						   (i < this->best_natt_ext || this->best_natt_ext < 0))
						{
							this->best_natt_ext = i;
						}
						found = TRUE;
						break;
					}
				}
			}
			if (!found)
			{
				DBG1(DBG_ENC, "received unknown vendor ID: %#B", &data);
			}
		}
	}
	enumerator->destroy(enumerator);

	if (this->best_natt_ext >= 0)
	{
		this->ike_sa->enable_extension(this->ike_sa,
								vendor_natt_ids[this->best_natt_ext].extension);
	}
}

METHOD(task_t, build_i, status_t,
	private_isakmp_vendor_t *this, message_t *message)
{
	if (this->count++ == 0)
	{
		build(this, message);
	}
	if (message->get_exchange_type(message) == AGGRESSIVE && this->count > 1)
	{
		return SUCCESS;
	}
	return NEED_MORE;
}

METHOD(task_t, process_r, status_t,
	private_isakmp_vendor_t *this, message_t *message)
{
	this->count++;
	process(this, message);
	if (message->get_exchange_type(message) == AGGRESSIVE && this->count > 1)
	{
		return SUCCESS;
	}
	return NEED_MORE;
}

METHOD(task_t, build_r, status_t,
	private_isakmp_vendor_t *this, message_t *message)
{
	if (this->count == 1)
	{
		build(this, message);
	}
	if (message->get_exchange_type(message) == ID_PROT && this->count > 2)
	{
		return SUCCESS;
	}
	return NEED_MORE;
}

METHOD(task_t, process_i, status_t,
	private_isakmp_vendor_t *this, message_t *message)
{
	process(this, message);
	if (message->get_exchange_type(message) == ID_PROT && this->count > 2)
	{
		return SUCCESS;
	}
	return NEED_MORE;
}

METHOD(task_t, migrate, void,
	private_isakmp_vendor_t *this, ike_sa_t *ike_sa)
{
	this->ike_sa = ike_sa;
	this->count = 0;
}

METHOD(task_t, get_type, task_type_t,
	private_isakmp_vendor_t *this)
{
	return TASK_ISAKMP_VENDOR;
}

METHOD(task_t, destroy, void,
	private_isakmp_vendor_t *this)
{
	free(this);
}

/**
 * See header
 */
isakmp_vendor_t *isakmp_vendor_create(ike_sa_t *ike_sa, bool initiator)
{
	private_isakmp_vendor_t *this;

	INIT(this,
		.public = {
			.task = {
				.migrate = _migrate,
				.get_type = _get_type,
				.destroy = _destroy,
			},
		},
		.initiator = initiator,
		.ike_sa = ike_sa,
		.best_natt_ext = -1,
	);

	if (initiator)
	{
		this->public.task.build = _build_i;
		this->public.task.process = _process_i;
	}
	else
	{
		this->public.task.build = _build_r;
		this->public.task.process = _process_r;
	}

	return &this->public;
}
