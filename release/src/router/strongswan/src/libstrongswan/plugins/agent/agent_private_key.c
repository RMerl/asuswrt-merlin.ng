/*
 * Copyright (C) 2013-2019 Tobias Brunner
 * Copyright (C) 2008-2009 Martin Willi
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

#include "agent_private_key.h"

#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <arpa/inet.h>
#include <errno.h>

#include <library.h>
#include <utils/chunk.h>
#include <utils/debug.h>

#ifndef UNIX_PATH_MAX
#define UNIX_PATH_MAX 108
#endif /* UNIX_PATH_MAX */

typedef struct private_agent_private_key_t private_agent_private_key_t;
typedef enum agent_msg_type_t agent_msg_type_t;

/**
 * Private data of a agent_private_key_t object.
 */
struct private_agent_private_key_t {
	/**
	 * Public interface for this signer.
	 */
	agent_private_key_t public;

	/**
	 * Path to the UNIX socket
	 */
	char *path;

	/**
	 * public key encoded in SSH format
	 */
	chunk_t key;

	/**
	 * public key
	 */
	public_key_t *pubkey;

	/**
	 * keysize in bytes
	 */
	size_t key_size;

	/**
	 * reference count
	 */
	refcount_t ref;
};

/**
 * Message types for ssh-agent protocol
 */
enum agent_msg_type_t {
	SSH_AGENT_FAILURE = 5,
	SSH_AGENT_SUCCESS =	6,
	SSH_AGENT_ID_REQUEST = 11,
	SSH_AGENT_ID_RESPONSE = 12,
	SSH_AGENT_SIGN_REQUEST = 13,
	SSH_AGENT_SIGN_RESPONSE = 14,
};

/**
 * Flags for signatures
 */
enum agent_signature_flags_t {
	SSH_AGENT_FLAG_SHA2_256 = 2,
	SSH_AGENT_FLAG_SHA2_512 = 4,
};

/**
 * read a byte from a blob
 */
static u_char read_byte(chunk_t *blob)
{
	u_char val;

	if (blob->len < sizeof(u_char))
	{
		return 0;
	}
	val = *(blob->ptr);
	*blob = chunk_skip(*blob, sizeof(u_char));
	return val;
}

/**
 * read a uint32_t from a blob
 */
static uint32_t read_uint32(chunk_t *blob)
{
	uint32_t val;

	if (blob->len < sizeof(uint32_t))
	{
		return 0;
	}
	val = ntohl(*(uint32_t*)blob->ptr);
	*blob = chunk_skip(*blob, sizeof(uint32_t));
	return val;
}

/**
 * read a ssh-agent "string" length/value from a blob
 */
static chunk_t read_string(chunk_t *blob)
{
	int len;
	chunk_t str;

	len = read_uint32(blob);
	if (len > blob->len)
	{
		return chunk_empty;
	}
	str = chunk_create(blob->ptr, len);
	*blob = chunk_skip(*blob, + len);
	return str;
}

/**
 * open socket connection to the ssh-agent
 */
static int open_connection(char *path)
{
	struct sockaddr_un addr;
	int s;

	s = socket(AF_UNIX, SOCK_STREAM, 0);
	if (s == -1)
	{
		DBG1(DBG_LIB, "opening ssh-agent socket %s failed: %s:", path,
			 strerror(errno));
		return -1;
	}

	addr.sun_family = AF_UNIX;
	addr.sun_path[UNIX_PATH_MAX - 1] = '\0';
	strncpy(addr.sun_path, path, UNIX_PATH_MAX - 1);

	if (connect(s, (struct sockaddr*)&addr, SUN_LEN(&addr)) != 0)
	{
		DBG1(DBG_LIB, "connecting to ssh-agent socket '%s' failed: %s",
			 addr.sun_path, strerror(errno));
		close(s);
		return -1;
	}
	return s;
}

/**
 * Get the first usable key from the agent
 */
static bool read_key(private_agent_private_key_t *this, public_key_t *pubkey)
{
	int socket, len;
	char buf[2048];
	chunk_t blob, key;
	bool success = FALSE;

	socket = open_connection(this->path);
	if (socket < 0)
	{
		return FALSE;
	}

	len = htonl(1);
	buf[0] = SSH_AGENT_ID_REQUEST;
	if (write(socket, &len, sizeof(len)) != sizeof(len) ||
		write(socket, &buf, 1) != 1)
	{
		DBG1(DBG_LIB, "writing to ssh-agent failed");
		goto done;
	}

	blob = chunk_create(buf, sizeof(buf));
	blob.len = read(socket, blob.ptr, blob.len);

	if (blob.len < sizeof(uint32_t) + sizeof(u_char) ||
		read_uint32(&blob) != blob.len ||
		read_byte(&blob) != SSH_AGENT_ID_RESPONSE)
	{
		DBG1(DBG_LIB, "received invalid ssh-agent identity response");
		goto done;
	}
	read_uint32(&blob);

	while (blob.len)
	{
		key = read_string(&blob);
		if (!key.len)
		{
			break;
		}
		this->pubkey = lib->creds->create(lib->creds, CRED_PUBLIC_KEY, KEY_ANY,
										  BUILD_BLOB_SSHKEY, key, BUILD_END);
		if (!this->pubkey)
		{
			continue;
		}
		if (pubkey && !private_key_belongs_to(&this->public.key, pubkey))
		{
			this->pubkey->destroy(this->pubkey);
			this->pubkey = NULL;
			continue;
		}
		this->key = chunk_clone(key);
		success = TRUE;
		break;
	}
done:
	close(socket);
	return success;
}

static bool scheme_supported(private_agent_private_key_t *this,
							 signature_scheme_t scheme, uint32_t *flags,
							 char **prefix)
{
	switch (this->pubkey->get_type(this->pubkey))
	{
		case KEY_RSA:
			switch (scheme)
			{
				case SIGN_RSA_EMSA_PKCS1_SHA1:
					*prefix = "ssh-rsa";
					return TRUE;
				case SIGN_RSA_EMSA_PKCS1_SHA2_256:
					*flags |= SSH_AGENT_FLAG_SHA2_256;
					*prefix = "rsa-sha2-256";
					return TRUE;
				case SIGN_RSA_EMSA_PKCS1_SHA2_512:
					*flags |= SSH_AGENT_FLAG_SHA2_512;
					*prefix = "rsa-sha2-512";
					return TRUE;
				default:
					break;
			}
			return FALSE;
		case KEY_ED25519:
			*prefix = "ssh-ed25519";
			return scheme == SIGN_ED25519;
		case KEY_ED448:
			*prefix = "ssh-ed448";
			return scheme == SIGN_ED448;
		case KEY_ECDSA:
			return scheme == SIGN_ECDSA_256 ||
				   scheme == SIGN_ECDSA_384 ||
				   scheme == SIGN_ECDSA_521;
		default:
			return FALSE;
	}
}

METHOD(private_key_t, sign, bool,
	private_agent_private_key_t *this, signature_scheme_t scheme, void *params,
	chunk_t data, chunk_t *signature)
{
	key_type_t type;
	uint32_t len, flags = 0;
	char buf[2048], *prefix = NULL;
	chunk_t blob;
	int socket;
	bool success = FALSE;

	if (!scheme_supported(this, scheme, &flags, &prefix))
	{
		DBG1(DBG_LIB, "signature scheme %N not supported by ssh-agent",
			 signature_scheme_names, scheme);
		return FALSE;
	}

	socket = open_connection(this->path);
	if (socket < 0)
	{
		return FALSE;
	}

	len = htonl(1 + sizeof(uint32_t) * 3 + this->key.len + data.len);
	buf[0] = SSH_AGENT_SIGN_REQUEST;
	if (write(socket, &len, sizeof(len)) != sizeof(len) ||
		write(socket, &buf, 1) != 1)
	{
		DBG1(DBG_LIB, "writing to ssh-agent failed");
		goto done;
	}

	len = htonl(this->key.len);
	if (write(socket, &len, sizeof(len)) != sizeof(len) ||
		write(socket, this->key.ptr, this->key.len) != this->key.len)
	{
		DBG1(DBG_LIB, "writing to ssh-agent failed");
		goto done;
	}

	len = htonl(data.len);
	if (write(socket, &len, sizeof(len)) != sizeof(len) ||
		write(socket, data.ptr, data.len) != data.len)
	{
		DBG1(DBG_LIB, "writing to ssh-agent failed");
		goto done;
	}

	flags = htonl(flags);
	if (write(socket, &flags, sizeof(flags)) != sizeof(flags))
	{
		DBG1(DBG_LIB, "writing to ssh-agent failed");
		goto done;
	}

	blob = chunk_create(buf, sizeof(buf));
	blob.len = read(socket, blob.ptr, blob.len);
	if (blob.len < sizeof(uint32_t) + sizeof(u_char) ||
		read_uint32(&blob) != blob.len ||
		read_byte(&blob) != SSH_AGENT_SIGN_RESPONSE)
	{
		DBG1(DBG_LIB, "received invalid ssh-agent signature response");
		goto done;
	}
	/* parse length */
	blob = read_string(&blob);
	/* verify type */
	if (prefix && !chunk_equals(read_string(&blob), chunk_from_str(prefix)))
	{
		DBG1(DBG_LIB, "ssh-agent didn't return requested %s signature", prefix);
		goto done;
	}
	type = this->pubkey->get_type(this->pubkey);
	if (type == KEY_RSA || type == KEY_ED25519 || type == KEY_ED448)
	{	/* for RSA/EdDSA, the signature has no special encoding */
		blob = read_string(&blob);
		if (blob.len)
		{
			*signature = chunk_clone(blob);
			success = TRUE;
		}
	}
	else
	{	/* parse ECDSA signatures */
		blob = read_string(&blob);
		if (blob.len)
		{
			chunk_t r, s;

			r = read_string(&blob);
			s = read_string(&blob);
			if (r.len && s.len)
			{
				*signature = chunk_cat("cc", r, s);
				success = TRUE;
			}
		}
	}
	if (!success)
	{
		DBG1(DBG_LIB, "received invalid ssh-agent signature response");
	}

done:
	close(socket);
	return success;
}

METHOD(private_key_t, get_type, key_type_t,
	private_agent_private_key_t *this)
{
	return this->pubkey->get_type(this->pubkey);
}

METHOD(private_key_t, decrypt, bool,
	private_agent_private_key_t *this, encryption_scheme_t scheme,
	void *params, chunk_t crypto, chunk_t *plain)
{
	DBG1(DBG_LIB, "private key decryption not supported by ssh-agent");
	return FALSE;
}

METHOD(private_key_t, get_keysize, int,
	private_agent_private_key_t *this)
{
	return this->pubkey->get_keysize(this->pubkey);
}

/**
 * Private data for RSA scheme enumerator
 */
typedef struct {
	enumerator_t public;
	int index;
	bool reverse;
} scheme_enumerator_t;

static signature_params_t rsa_schemes[] = {
	{ .scheme = SIGN_RSA_EMSA_PKCS1_SHA2_256 },
	{ .scheme = SIGN_RSA_EMSA_PKCS1_SHA2_512 },
};

METHOD(enumerator_t, enumerate_rsa_scheme, bool,
	scheme_enumerator_t *this, va_list args)
{
	signature_params_t **params;

	VA_ARGS_VGET(args, params);

	if ((this->reverse && --this->index >= 0) ||
	   (!this->reverse && ++this->index < countof(rsa_schemes)))
	{
		*params = &rsa_schemes[this->index];
		return TRUE;
	}
	return FALSE;
}

/**
 * Create an enumerator for the supported RSA signature schemes
 */
static enumerator_t *create_rsa_enumerator(private_agent_private_key_t *this)
{
	scheme_enumerator_t *enumerator;

	INIT(enumerator,
		.public = {
			.enumerate = enumerator_enumerate_default,
			.venumerate = _enumerate_rsa_scheme,
			.destroy = (void*)free,
		},
		.index = -1,
		.reverse = FALSE,
	);
	/* propose SHA-512 first for larger keys */
	if (get_keysize(this) > 3072)
	{
		enumerator->index = countof(rsa_schemes);
		enumerator->reverse = TRUE;
	}
	return &enumerator->public;
}

METHOD(private_key_t, supported_signature_schemes, enumerator_t*,
	private_agent_private_key_t *this)
{
	key_type_t type = get_type(this);

	switch (type)
	{
		case KEY_RSA:
			return create_rsa_enumerator(this);
		case KEY_ED25519:
		case KEY_ED448:
		case KEY_ECDSA:
			return signature_schemes_for_key(type, get_keysize(this));
		default:
			break;
	}
	return enumerator_create_empty();
}

METHOD(private_key_t, get_public_key, public_key_t*,
	private_agent_private_key_t *this)
{
	return this->pubkey->get_ref(this->pubkey);
}

METHOD(private_key_t, get_encoding, bool,
	private_agent_private_key_t *this, cred_encoding_type_t type,
	chunk_t *encoding)
{
	return FALSE;
}

METHOD(private_key_t, get_fingerprint, bool,
	private_agent_private_key_t *this, cred_encoding_type_t type, chunk_t *fp)
{
	return this->pubkey->get_fingerprint(this->pubkey, type, fp);
}

METHOD(private_key_t, get_ref, private_key_t*,
	private_agent_private_key_t *this)
{
	ref_get(&this->ref);
	return &this->public.key;
}

METHOD(private_key_t, destroy, void,
	private_agent_private_key_t *this)
{
	if (ref_put(&this->ref))
	{
		chunk_free(&this->key);
		DESTROY_IF(this->pubkey);
		free(this->path);
		free(this);
	}
}

/**
 * See header.
 */
agent_private_key_t *agent_private_key_open(key_type_t type, va_list args)
{
	private_agent_private_key_t *this;
	public_key_t *pubkey = NULL;
	char *path = NULL;

	while (TRUE)
	{
		switch (va_arg(args, builder_part_t))
		{
			case BUILD_AGENT_SOCKET:
				path = va_arg(args, char*);
				continue;
			case BUILD_PUBLIC_KEY:
				pubkey = va_arg(args, public_key_t*);
				continue;
			case BUILD_END:
				break;
			default:
				return NULL;
		}
		break;
	}
	if (!path)
	{
		return NULL;
	}

	INIT(this,
		.public = {
			.key = {
				.get_type = _get_type,
				.supported_signature_schemes = _supported_signature_schemes,
				.sign = _sign,
				.decrypt = _decrypt,
				.get_keysize = _get_keysize,
				.get_public_key = _get_public_key,
				.belongs_to = private_key_belongs_to,
				.equals = private_key_equals,
				.get_fingerprint = _get_fingerprint,
				.has_fingerprint = private_key_has_fingerprint,
				.get_encoding = _get_encoding,
				.get_ref = _get_ref,
				.destroy = _destroy,
			},
		},
		.path = strdup(path),
		.ref = 1,
	);

	if (!read_key(this, pubkey))
	{
		destroy(this);
		return NULL;
	}
	return &this->public;
}
