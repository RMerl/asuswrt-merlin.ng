/*
 * Copyright (C) 2009 Martin Willi
 * Hochschule fuer Technik Rapperswil
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

#include "radius_message.h"

#include <utils/debug.h>
#include <bio/bio_reader.h>
#include <crypto/hashers/hasher.h>

typedef struct private_radius_message_t private_radius_message_t;
typedef struct rmsg_t rmsg_t;
typedef struct rattr_t rattr_t;

/**
 * RADIUS message header
 */
struct rmsg_t {
	/** message code, radius_message_code_t */
	u_int8_t code;
	/** message identifier */
	u_int8_t identifier;
	/** length of Code, Identifier, Length, Authenticator and Attributes */
	u_int16_t length;
	/** message authenticator, MD5 hash */
	u_int8_t authenticator[HASH_SIZE_MD5];
	/** variable list of packed attributes */
	u_int8_t attributes[];
} __attribute__((packed));

/**
 * RADIUS message attribute.
 */
struct rattr_t {
	/** attribute type, radius_attribute_type_t */
	u_int8_t type;
	/** length of the attriubte, including the Type, Length and Value fields */
	u_int8_t length;
	/** variable length attribute value */
	u_int8_t value[];
} __attribute__((packed));

/**
 * Private data of an radius_message_t object.
 */
struct private_radius_message_t {

	/**
	 * Public radius_message_t interface.
	 */
	radius_message_t public;

	/**
	 * message data, allocated
	 */
	rmsg_t *msg;

	/**
	 * User-Password to encrypt and encode, if any
	 */
	chunk_t password;
};

/**
 * Described in header.
 */
void libradius_init(void)
{
	/* empty */
}

ENUM_BEGIN(radius_message_code_names, RMC_ACCESS_REQUEST, RMC_ACCOUNTING_RESPONSE,
	"Access-Request",
	"Access-Accept",
	"Access-Reject",
	"Accounting-Request",
	"Accounting-Response");
ENUM_NEXT(radius_message_code_names, RMC_ACCESS_CHALLENGE, RMC_ACCESS_CHALLENGE, RMC_ACCOUNTING_RESPONSE,
	"Access-Challenge");
ENUM_NEXT(radius_message_code_names, RMC_DISCONNECT_REQUEST, RMC_COA_NAK, RMC_ACCESS_CHALLENGE,
	"Disconnect-Request",
	"Disconnect-ACK",
	"Disconnect-NAK",
	"CoA-Request",
	"CoA-ACK",
	"CoA-NAK");
ENUM_END(radius_message_code_names, RMC_COA_NAK);

ENUM(radius_attribute_type_names, RAT_USER_NAME, RAT_MIP6_HOME_LINK_PREFIX,
	"User-Name",
	"User-Password",
	"CHAP-Password",
	"NAS-IP-Address",
	"NAS-Port",
	"Service-Type",
	"Framed-Protocol",
	"Framed-IP-Address",
	"Framed-IP-Netmask",
	"Framed-Routing",
	"Filter-Id",
	"Framed-MTU",
	"Framed-Compression",
	"Login-IP-Host",
	"Login-Service",
	"Login-TCP-Port",
	"Unassigned",
	"Reply-Message",
	"Callback-Number",
	"Callback-Id",
	"Unassigned",
	"Framed-Route",
	"Framed-IPX-Network",
	"State",
	"Class",
	"Vendor-Specific",
	"Session-Timeout",
	"Idle-Timeout",
	"Termination-Action",
	"Called-Station-Id",
	"Calling-Station-Id",
	"NAS-Identifier",
	"Proxy-State",
	"Login-LAT-Service",
	"Login-LAT-Node",
	"Login-LAT-Group",
	"Framed-AppleTalk-Link",
	"Framed-AppleTalk-Network",
	"Framed-AppleTalk-Zone",
	"Acct-Status-Type",
	"Acct-Delay-Time",
	"Acct-Input-Octets",
	"Acct-Output-Octets",
	"Acct-Session-Id",
	"Acct-Authentic",
	"Acct-Session-Time",
	"Acct-Input-Packets",
	"Acct-Output-Packets",
	"Acct-Terminate-Cause",
	"Acct-Multi-Session-Id",
	"Acct-Link-Count",
	"Acct-Input-Gigawords",
	"Acct-Output-Gigawords",
	"Unassigned",
	"Event-Timestamp",
	"Egress-VLANID",
	"Ingress-Filters",
	"Egress-VLAN-Name",
	"User-Priority-Table",
	"CHAP-Challenge",
	"NAS-Port-Type",
	"Port-Limit",
	"Login-LAT-Port",
	"Tunnel-Type",
	"Tunnel-Medium-Type",
	"Tunnel-Client-Endpoint",
	"Tunnel-Server-Endpoint",
	"Acct-Tunnel-Connection",
	"Tunnel-Password",
	"ARAP-Password",
	"ARAP-Features",
	"ARAP-Zone-Access",
	"ARAP-Security",
	"ARAP-Security-Data",
	"Password-Retry",
	"Prompt",
	"Connect-Info",
	"Configuration-Token",
	"EAP-Message",
	"Message-Authenticator",
	"Tunnel-Private-Group-ID",
	"Tunnel-Assignment-ID",
	"Tunnel-Preference",
	"ARAP-Challenge-Response",
	"Acct-Interim-Interval",
	"Acct-Tunnel-Packets-Lost",
	"NAS-Port-Id",
	"Framed-Pool",
	"CUI",
	"Tunnel-Client-Auth-ID",
	"Tunnel-Server-Auth-ID",
	"NAS-Filter-Rule",
	"Unassigned",
	"Originating-Line-Info",
	"NAS-IPv6-Address",
	"Framed-Interface-Id",
	"Framed-IPv6-Prefix",
	"Login-IPv6-Host",
	"Framed-IPv6-Route",
	"Framed-IPv6-Pool",
	"Error-Cause",
	"EAP-Key-Name",
	"Digest-Response",
	"Digest-Realm",
	"Digest-Nonce",
	"Digest-Response-Auth",
	"Digest-Nextnonce",
	"Digest-Method",
	"Digest-URI",
	"Digest-Qop",
	"Digest-Algorithm",
	"Digest-Entity-Body-Hash",
	"Digest-CNonce",
	"Digest-Nonce-Count",
	"Digest-Username",
	"Digest-Opaque",
	"Digest-Auth-Param",
	"Digest-AKA-Auts",
	"Digest-Domain",
	"Digest-Stale",
	"Digest-HA1",
	"SIP-AOR",
	"Delegated-IPv6-Prefix",
	"MIP6-Feature-Vector",
	"MIP6-Home-Link-Prefix");

/**
 * Attribute enumerator implementation
 */
typedef struct {
	/** implements enumerator interface */
	enumerator_t public;
	/** currently pointing attribute */
	rattr_t *next;
	/** bytes left */
	int left;
} attribute_enumerator_t;

METHOD(enumerator_t, attribute_enumerate, bool,
	attribute_enumerator_t *this, int *type, chunk_t *data)
{
	if (this->left == 0)
	{
		return FALSE;
	}
	if (this->left < sizeof(rattr_t) ||
		this->left < this->next->length)
	{
		DBG1(DBG_IKE, "RADIUS message truncated");
		return FALSE;
	}
	*type = this->next->type;
	data->ptr = this->next->value;
	data->len = this->next->length - sizeof(rattr_t);
	this->left -= this->next->length;
	this->next = ((void*)this->next) + this->next->length;
	return TRUE;
}

METHOD(radius_message_t, create_enumerator, enumerator_t*,
	private_radius_message_t *this)
{
	attribute_enumerator_t *e;

	if (ntohs(this->msg->length) < sizeof(rmsg_t) + sizeof(rattr_t))
	{
		return enumerator_create_empty();
	}
	INIT(e,
		.public = {
			.enumerate = (void*)_attribute_enumerate,
			.destroy = (void*)free,
		},
		.next = (rattr_t*)this->msg->attributes,
		.left = ntohs(this->msg->length) - sizeof(rmsg_t),
	);
	return &e->public;
}

/**
 * Vendor attribute enumerator implementation
 */
typedef struct {
	/** implements enumerator interface */
	enumerator_t public;
	/** inner attribute enumerator */
	enumerator_t *inner;
	/** current vendor ID */
	u_int32_t vendor;
	/** reader for current vendor ID */
	bio_reader_t *reader;
} vendor_enumerator_t;

METHOD(enumerator_t, vendor_enumerate, bool,
	vendor_enumerator_t *this, int *vendor, int *type, chunk_t *data)
{
	chunk_t inner_data;
	int inner_type;
	u_int8_t type8, len;

	while (TRUE)
	{
		if (this->reader)
		{
			if (this->reader->remaining(this->reader) >= 2 &&
				this->reader->read_uint8(this->reader, &type8) &&
				this->reader->read_uint8(this->reader, &len) && len >= 2 &&
				this->reader->read_data(this->reader, len - 2, data))
			{
				*vendor = this->vendor;
				*type = type8;
				return TRUE;
			}
			this->reader->destroy(this->reader);
			this->reader = NULL;
		}
		if (this->inner->enumerate(this->inner, &inner_type, &inner_data))
		{
			if (inner_type == RAT_VENDOR_SPECIFIC)
			{
				this->reader = bio_reader_create(inner_data);
				if (!this->reader->read_uint32(this->reader, &this->vendor))
				{
					this->reader->destroy(this->reader);
					this->reader = NULL;
				}
			}
		}
		else
		{
			return FALSE;
		}
	}
}
METHOD(enumerator_t, vendor_destroy, void,
	vendor_enumerator_t *this)
{
	DESTROY_IF(this->reader);
	this->inner->destroy(this->inner);
	free(this);
}

METHOD(radius_message_t, create_vendor_enumerator, enumerator_t*,
	private_radius_message_t *this)
{
	vendor_enumerator_t *e;

	INIT(e,
		.public = {
			.enumerate = (void*)_vendor_enumerate,
			.destroy = _vendor_destroy,
		},
		.inner = create_enumerator(this),
	);

	return &e->public;
}

METHOD(radius_message_t, add, void,
	private_radius_message_t *this, radius_attribute_type_t type, chunk_t data)
{
	rattr_t *attribute;

	if (type == RAT_USER_PASSWORD && !this->password.len)
	{
		/* store a null-padded password */
		this->password = chunk_alloc(round_up(data.len, HASH_SIZE_MD5));
		memset(this->password.ptr + data.len, 0, this->password.len - data.len);
		memcpy(this->password.ptr, data.ptr, data.len);
		return;
	}

	data.len = min(data.len, MAX_RADIUS_ATTRIBUTE_SIZE);
	this->msg = realloc(this->msg,
						ntohs(this->msg->length) + sizeof(rattr_t) + data.len);
	attribute = ((void*)this->msg) + ntohs(this->msg->length);
	attribute->type = type;
	attribute->length = data.len + sizeof(rattr_t);
	memcpy(attribute->value, data.ptr, data.len);
	this->msg->length = htons(ntohs(this->msg->length) + attribute->length);
}

METHOD(radius_message_t, crypt, bool,
	private_radius_message_t *this, chunk_t salt, chunk_t in, chunk_t out,
	chunk_t secret, hasher_t *hasher)
{
	char b[HASH_SIZE_MD5];

	/**
	 * From RFC2548 (encryption):
	 * b(1) = MD5(S + R + A)    c(1) = p(1) xor b(1)   C = c(1)
	 * b(2) = MD5(S + c(1))     c(2) = p(2) xor b(2)   C = C + c(2)
	 *      . . .
	 * b(i) = MD5(S + c(i-1))   c(i) = p(i) xor b(i)   C = C + c(i)
	 *
	 * P/C = Plain/Crypted => in/out
	 * S = secret
	 * R = authenticator
	 * A = salt
	 */
	if (in.len != out.len)
	{
		return FALSE;
	}
	if (in.len % HASH_SIZE_MD5 || in.len < HASH_SIZE_MD5)
	{
		return FALSE;
	}
	if (out.ptr != in.ptr)
	{
		memcpy(out.ptr, in.ptr, in.len);
	}
	/* Preparse seed for first round:
	 * b(1) = MD5(S + R + A) */
	if (!hasher->get_hash(hasher, secret, NULL) ||
		!hasher->get_hash(hasher,
						  chunk_from_thing(this->msg->authenticator), NULL) ||
		!hasher->get_hash(hasher, salt, b))
	{
		return FALSE;
	}
	while (in.len)
	{
		/* p(i) = b(i) xor c(1) */
		memxor(out.ptr, b, HASH_SIZE_MD5);

		out = chunk_skip(out, HASH_SIZE_MD5);
		if (out.len)
		{
			/* Prepare seed for next round::
			 * b(i) = MD5(S + c(i-1)) */
			if (!hasher->get_hash(hasher, secret, NULL) ||
				!hasher->get_hash(hasher,
								  chunk_create(in.ptr, HASH_SIZE_MD5), b))
			{
				return FALSE;
			}
		}
		in = chunk_skip(in, HASH_SIZE_MD5);
	}
	return TRUE;
}

METHOD(radius_message_t, sign, bool,
	private_radius_message_t *this, u_int8_t *req_auth, chunk_t secret,
	hasher_t *hasher, signer_t *signer, rng_t *rng, bool msg_auth)
{
	if (rng)
	{
		/* build Request-Authenticator */
		if (!rng->get_bytes(rng, HASH_SIZE_MD5, this->msg->authenticator))
		{
			return FALSE;
		}
	}
	else
	{
		/* prepare build of Response-Authenticator */
		if (req_auth)
		{
			memcpy(this->msg->authenticator, req_auth, HASH_SIZE_MD5);
		}
		else
		{
			memset(this->msg->authenticator, 0, sizeof(this->msg->authenticator));
		}
	}

	if (this->password.len)
	{
		/* encrypt password inline */
		if (!crypt(this, chunk_empty, this->password, this->password,
				   secret, hasher))
		{
			return FALSE;
		}
		add(this, RAT_USER_PASSWORD, this->password);
		chunk_clear(&this->password);
	}

	if (msg_auth)
	{
		char buf[HASH_SIZE_MD5];

		/* build Message-Authenticator attribute, using 16 null bytes */
		memset(buf, 0, sizeof(buf));
		add(this, RAT_MESSAGE_AUTHENTICATOR, chunk_create(buf, sizeof(buf)));
		if (!signer->get_signature(signer,
				chunk_create((u_char*)this->msg, ntohs(this->msg->length)),
				((u_char*)this->msg) + ntohs(this->msg->length) - HASH_SIZE_MD5))
		{
			return FALSE;
		}
	}

	if (!rng)
	{
		chunk_t msg;

		/* build Response-Authenticator */
		msg = chunk_create((u_char*)this->msg, ntohs(this->msg->length));
		if (!hasher->get_hash(hasher, msg, NULL) ||
			!hasher->get_hash(hasher, secret, this->msg->authenticator))
		{
			return FALSE;
		}
	}
	return TRUE;
}

METHOD(radius_message_t, verify, bool,
	private_radius_message_t *this, u_int8_t *req_auth, chunk_t secret,
	hasher_t *hasher, signer_t *signer)
{
	char buf[HASH_SIZE_MD5], res_auth[HASH_SIZE_MD5];
	enumerator_t *enumerator;
	int type;
	chunk_t data, msg;
	bool has_eap = FALSE, has_auth = FALSE;

	msg = chunk_create((u_char*)this->msg, ntohs(this->msg->length));

	if (this->msg->code != RMC_ACCESS_REQUEST)
	{
		/* replace Response by Request Authenticator for verification */
		memcpy(res_auth, this->msg->authenticator, HASH_SIZE_MD5);
		if (req_auth)
		{
			memcpy(this->msg->authenticator, req_auth, HASH_SIZE_MD5);
		}
		else
		{
			memset(this->msg->authenticator, 0, HASH_SIZE_MD5);
		}

		/* verify Response-Authenticator */
		if (!hasher->get_hash(hasher, msg, NULL) ||
			!hasher->get_hash(hasher, secret, buf) ||
			!memeq(buf, res_auth, HASH_SIZE_MD5))
		{
			DBG1(DBG_CFG, "RADIUS Response-Authenticator verification failed");
			return FALSE;
		}
	}

	/* verify Message-Authenticator attribute */
	enumerator = create_enumerator(this);
	while (enumerator->enumerate(enumerator, &type, &data))
	{
		if (type == RAT_MESSAGE_AUTHENTICATOR)
		{
			if (data.len != HASH_SIZE_MD5)
			{
				DBG1(DBG_CFG, "RADIUS Message-Authenticator invalid length");
				enumerator->destroy(enumerator);
				return FALSE;
			}
			memcpy(buf, data.ptr, data.len);
			memset(data.ptr, 0, data.len);
			if (signer->verify_signature(signer, msg,
										 chunk_create(buf, sizeof(buf))))
			{
				/* restore Message-Authenticator */
				memcpy(data.ptr, buf, data.len);
				has_auth = TRUE;
				break;
			}
			else
			{
				DBG1(DBG_CFG, "RADIUS Message-Authenticator verification failed");
				enumerator->destroy(enumerator);
				return FALSE;
			}
		}
		else if (type == RAT_EAP_MESSAGE)
		{
			has_eap = TRUE;
		}
	}
	enumerator->destroy(enumerator);

	if (this->msg->code != RMC_ACCESS_REQUEST)
	{
		/* restore Response-Authenticator */
		memcpy(this->msg->authenticator, res_auth, HASH_SIZE_MD5);
	}

	if (has_eap && !has_auth)
	{	/* Message-Authenticator is required if we have an EAP-Message */
		DBG1(DBG_CFG, "RADIUS Message-Authenticator attribute missing");
		return FALSE;
	}
	return TRUE;
}

METHOD(radius_message_t, get_code, radius_message_code_t,
	private_radius_message_t *this)
{
	return this->msg->code;
}

METHOD(radius_message_t, get_identifier, u_int8_t,
	private_radius_message_t *this)
{
	return this->msg->identifier;
}

METHOD(radius_message_t, set_identifier, void,
	private_radius_message_t *this, u_int8_t identifier)
{
	this->msg->identifier = identifier;
}

METHOD(radius_message_t, get_authenticator, u_int8_t*,
	private_radius_message_t *this)
{
	return this->msg->authenticator;
}


METHOD(radius_message_t, get_encoding, chunk_t,
	private_radius_message_t *this)
{
	return chunk_create((u_char*)this->msg, ntohs(this->msg->length));
}

METHOD(radius_message_t, destroy, void,
	private_radius_message_t *this)
{
	chunk_clear(&this->password);
	free(this->msg);
	free(this);
}

/**
 * Generic constructor
 */
static private_radius_message_t *radius_message_create_empty()
{
	private_radius_message_t *this;

	INIT(this,
		.public = {
			.create_enumerator = _create_enumerator,
			.create_vendor_enumerator = _create_vendor_enumerator,
			.add = _add,
			.get_code = _get_code,
			.get_identifier = _get_identifier,
			.set_identifier = _set_identifier,
			.get_authenticator = _get_authenticator,
			.get_encoding = _get_encoding,
			.sign = _sign,
			.verify = _verify,
			.crypt = _crypt,
			.destroy = _destroy,
		},
	);

	return this;
}

/**
 * See header
 */
radius_message_t *radius_message_create(radius_message_code_t code)
{
	private_radius_message_t *this = radius_message_create_empty();

	INIT(this->msg,
		.code = code,
		.identifier = 0,
		.length = htons(sizeof(rmsg_t)),
	);

	return &this->public;
}

/**
 * See header
 */
radius_message_t *radius_message_parse(chunk_t data)
{
	private_radius_message_t *this = radius_message_create_empty();

	this->msg = malloc(data.len);
	memcpy(this->msg, data.ptr, data.len);
	if (data.len < sizeof(rmsg_t) ||
		ntohs(this->msg->length) != data.len)
	{
		DBG1(DBG_IKE, "RADIUS message has invalid length");
		destroy(this);
		return NULL;
	}
	return &this->public;
}
