/*
 * Copyright (C) 2016-2017 Tobias Brunner
 * Copyright (C) 2015 Andreas Steffen
 * HSR Hochschule fuer Technik Rapperswil
 *
 * Copyright (C) 2014 Martin Willi
 * Copyright (C) 2014 revosec AG
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

#define _GNU_SOURCE
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>

#include "command.h"
#include "swanctl.h"
#include "load_creds.h"

#include <credentials/sets/mem_cred.h>
#include <credentials/sets/callback_cred.h>
#include <credentials/containers/pkcs12.h>
#include <collections/hashtable.h>

#include <vici_cert_info.h>

/**
 * Context used to track loaded secrets
 */
typedef struct {
	/** vici connection */
	vici_conn_t *conn;
	/** format options */
	command_format_options_t format;
	/** read setting */
	settings_t *cfg;
	/** don't prompt user for password */
	bool noprompt;
	/** list of key ids of loaded private keys */
	hashtable_t *keys;
	/** list of unique ids of loaded shared keys */
	hashtable_t *shared;
} load_ctx_t;

/**
 * Load a single certificate over vici
 */
static bool load_cert(load_ctx_t *ctx, char *dir, certificate_type_t type,
					  x509_flag_t flag, chunk_t data)
{
	vici_req_t *req;
	vici_res_t *res;
	bool ret = TRUE;

	req = vici_begin("load-cert");

	vici_add_key_valuef(req, "type", "%N", certificate_type_names, type);
	if (type == CERT_X509)
	{
		vici_add_key_valuef(req, "flag", "%N", x509_flag_names, flag);
	}
	vici_add_key_value(req, "data", data.ptr, data.len);

	res = vici_submit(req, ctx->conn);
	if (!res)
	{
		fprintf(stderr, "load-cert request failed: %s\n", strerror(errno));
		return FALSE;
	}
	if (ctx->format & COMMAND_FORMAT_RAW)
	{
		vici_dump(res, "load-cert reply", ctx->format & COMMAND_FORMAT_PRETTY,
				  stdout);
	}
	else if (!streq(vici_find_str(res, "no", "success"), "yes"))
	{
		fprintf(stderr, "loading '%s' failed: %s\n",
				dir, vici_find_str(res, "", "errmsg"));
		ret = FALSE;
	}
	else
	{
		printf("loaded certificate from '%s'\n", dir);
	}
	vici_free_res(res);
	return ret;
}

/**
 * Load certficiates from a directory
 */
static void load_certs(load_ctx_t *ctx, char *type_str, char *dir)
{
	enumerator_t *enumerator;
	certificate_type_t type;
	x509_flag_t flag;
	struct stat st;
	chunk_t *map;
	char *path, buf[PATH_MAX];

	vici_cert_info_from_str(type_str, &type, &flag);

	snprintf(buf, sizeof(buf), "%s%s%s", swanctl_dir, DIRECTORY_SEPARATOR, dir);
	dir = buf;

	enumerator = enumerator_create_directory(dir);
	if (enumerator)
	{
		while (enumerator->enumerate(enumerator, NULL, &path, &st))
		{
			if (S_ISREG(st.st_mode))
			{
				map = chunk_map(path, FALSE);
				if (map)
				{
					load_cert(ctx, path, type, flag, *map);
					chunk_unmap(map);
				}
				else
				{
					fprintf(stderr, "mapping '%s' failed: %s, skipped\n",
							path, strerror(errno));
				}
			}
		}
		enumerator->destroy(enumerator);
	}
}

/**
 * Load a single private key over vici
 */
static bool load_key(load_ctx_t *ctx, char *dir, char *type, chunk_t data)
{
	vici_req_t *req;
	vici_res_t *res;
	bool ret = TRUE;
	char *id;

	req = vici_begin("load-key");

	if (streq(type, "private") ||
		streq(type, "pkcs8"))
	{	/* as used by vici */
		vici_add_key_valuef(req, "type", "any");
	}
	else
	{
		vici_add_key_valuef(req, "type", "%s", type);
	}
	vici_add_key_value(req, "data", data.ptr, data.len);

	res = vici_submit(req, ctx->conn);
	if (!res)
	{
		fprintf(stderr, "load-key request failed: %s\n", strerror(errno));
		return FALSE;
	}
	if (ctx->format & COMMAND_FORMAT_RAW)
	{
		vici_dump(res, "load-key reply", ctx->format & COMMAND_FORMAT_PRETTY,
				  stdout);
	}
	else if (!streq(vici_find_str(res, "no", "success"), "yes"))
	{
		fprintf(stderr, "loading '%s' failed: %s\n",
				dir, vici_find_str(res, "", "errmsg"));
		ret = FALSE;
	}
	else
	{
		printf("loaded %s key from '%s'\n", type, dir);
		id = vici_find_str(res, "", "id");
		free(ctx->keys->remove(ctx->keys, id));
	}
	vici_free_res(res);
	return ret;
}

/**
 * Load a private key of any type to vici
 */
static bool load_key_anytype(load_ctx_t *ctx, char *path,
							 private_key_t *private)
{
	bool loaded = FALSE;
	chunk_t encoding;

	if (!private->get_encoding(private, PRIVKEY_ASN1_DER, &encoding))
	{
		fprintf(stderr, "encoding private key from '%s' failed\n", path);
		return FALSE;
	}
	switch (private->get_type(private))
	{
		case KEY_RSA:
			loaded = load_key(ctx, path, "rsa", encoding);
			break;
		case KEY_ECDSA:
			loaded = load_key(ctx, path, "ecdsa", encoding);
			break;
		case KEY_BLISS:
			loaded = load_key(ctx, path, "bliss", encoding);
			break;
		default:
			fprintf(stderr, "unsupported key type in '%s'\n", path);
			break;
	}
	chunk_clear(&encoding);
	return loaded;
}

/**
 * Data passed to password callback
 */
typedef struct {
	char prompt[128];
	mem_cred_t *cache;
} cb_data_t;

/**
 * Callback function to prompt for private key passwords
 */
CALLBACK(password_cb, shared_key_t*,
	cb_data_t *data, shared_key_type_t type,
	identification_t *me, identification_t *other,
	id_match_t *match_me, id_match_t *match_other)
{
	shared_key_t *shared;
	char *pwd = NULL;

	if (type != SHARED_PRIVATE_KEY_PASS)
	{
		return NULL;
	}
#ifdef HAVE_GETPASS
	pwd = getpass(data->prompt);
#endif
	if (!pwd || strlen(pwd) == 0)
	{
		return NULL;
	}
	if (match_me)
	{
		*match_me = ID_MATCH_PERFECT;
	}
	if (match_other)
	{
		*match_other = ID_MATCH_PERFECT;
	}
	shared = shared_key_create(type, chunk_clone(chunk_from_str(pwd)));
	/* cache secret if it is required more than once (PKCS#12) */
	data->cache->add_shared(data->cache, shared, NULL);
	return shared->get_ref(shared);
}

/**
 * Determine credential type and subtype from a type string
 */
static bool determine_credtype(char *type, credential_type_t *credtype,
							   int *subtype)
{
	struct {
		char *type;
		credential_type_t credtype;
		int subtype;
	} map[] = {
		{ "private",		CRED_PRIVATE_KEY,		KEY_ANY,			},
		{ "pkcs8",			CRED_PRIVATE_KEY,		KEY_ANY,			},
		{ "rsa",			CRED_PRIVATE_KEY,		KEY_RSA,			},
		{ "ecdsa",			CRED_PRIVATE_KEY,		KEY_ECDSA,			},
		{ "bliss",			CRED_PRIVATE_KEY,		KEY_BLISS,			},
		{ "pkcs12",			CRED_CONTAINER,			CONTAINER_PKCS12,	},
	};
	int i;

	for (i = 0; i < countof(map); i++)
	{
		if (streq(map[i].type, type))
		{
			*credtype = map[i].credtype;
			*subtype = map[i].subtype;
			return TRUE;
		}
	}
	return FALSE;
}

/**
 * Try to parse a potentially encrypted credential using password prompt
 */
static void* decrypt(char *name, char *type, chunk_t encoding)
{
	credential_type_t credtype;
	int subtype;
	void *cred;
	callback_cred_t *cb;
	cb_data_t data;

	if (!determine_credtype(type, &credtype, &subtype))
	{
		return NULL;
	}

	snprintf(data.prompt, sizeof(data.prompt), "Password for %s file '%s': ",
			 type, name);

	data.cache = mem_cred_create();
	lib->credmgr->add_set(lib->credmgr, &data.cache->set);
	cb = callback_cred_create_shared(password_cb, &data);
	lib->credmgr->add_set(lib->credmgr, &cb->set);

	cred = lib->creds->create(lib->creds, credtype, subtype,
							  BUILD_BLOB_PEM, encoding, BUILD_END);

	lib->credmgr->remove_set(lib->credmgr, &data.cache->set);
	data.cache->destroy(data.cache);
	lib->credmgr->remove_set(lib->credmgr, &cb->set);
	cb->destroy(cb);

	return cred;
}

/**
 * Try to parse a potentially encrypted credential using configured secret
 */
static void* decrypt_with_config(load_ctx_t *ctx, char *name, char *type,
								 chunk_t encoding)
{
	credential_type_t credtype;
	int subtype;
	enumerator_t *enumerator, *secrets;
	char *section, *key, *value, *file;
	shared_key_t *shared;
	void *cred = NULL;
	mem_cred_t *mem = NULL;

	if (!determine_credtype(type, &credtype, &subtype))
	{
		return NULL;
	}

	/* load all secrets for this key type */
	enumerator = ctx->cfg->create_section_enumerator(ctx->cfg, "secrets");
	while (enumerator->enumerate(enumerator, &section))
	{
		if (strpfx(section, type))
		{
			file = ctx->cfg->get_str(ctx->cfg, "secrets.%s.file", NULL, section);
			if (file && strcaseeq(file, name))
			{
				secrets = ctx->cfg->create_key_value_enumerator(ctx->cfg,
														"secrets.%s", section);
				while (secrets->enumerate(secrets, &key, &value))
				{
					if (strpfx(key, "secret"))
					{
						if (!mem)
						{
							mem = mem_cred_create();
						}
						shared = shared_key_create(SHARED_PRIVATE_KEY_PASS,
											chunk_clone(chunk_from_str(value)));
						mem->add_shared(mem, shared, NULL);
					}
				}
				secrets->destroy(secrets);
			}
		}
	}
	enumerator->destroy(enumerator);

	if (mem)
	{
		lib->credmgr->add_local_set(lib->credmgr, &mem->set, FALSE);

		cred = lib->creds->create(lib->creds, credtype, subtype,
								  BUILD_BLOB_PEM, encoding, BUILD_END);

		lib->credmgr->remove_local_set(lib->credmgr, &mem->set);

		if (!cred)
		{
			fprintf(stderr, "configured decryption secret for '%s' invalid\n",
					name);
		}

		mem->destroy(mem);
	}

	return cred;
}

/**
 * Try to decrypt and load a private key
 */
static bool load_encrypted_key(load_ctx_t *ctx, char *rel, char *path,
							   char *type, chunk_t data)
{
	private_key_t *private;
	bool loaded = FALSE;

	private = decrypt_with_config(ctx, rel, type, data);
	if (!private && !ctx->noprompt)
	{
		private = decrypt(rel, type, data);
	}
	if (private)
	{
		loaded = load_key_anytype(ctx, path, private);
		private->destroy(private);
	}
	return loaded;
}

/**
 * Load private keys from a directory
 */
static void load_keys(load_ctx_t *ctx, char *type, char *dir)
{
	enumerator_t *enumerator;
	struct stat st;
	chunk_t *map;
	char *path, *rel, buf[PATH_MAX];

	snprintf(buf, sizeof(buf), "%s%s%s", swanctl_dir, DIRECTORY_SEPARATOR, dir);
	dir = buf;

	enumerator = enumerator_create_directory(dir);
	if (enumerator)
	{
		while (enumerator->enumerate(enumerator, &rel, &path, &st))
		{
			if (S_ISREG(st.st_mode))
			{
				map = chunk_map(path, FALSE);
				if (map)
				{
					if (!load_encrypted_key(ctx, rel, path, type, *map))
					{
						load_key(ctx, path, type, *map);
					}
					chunk_unmap(map);
				}
				else
				{
					fprintf(stderr, "mapping '%s' failed: %s, skipped\n",
							path, strerror(errno));
				}
			}
		}
		enumerator->destroy(enumerator);
	}
}

/**
 * Load credentials from a PKCS#12 container over vici
 */
static bool load_pkcs12(load_ctx_t *ctx, char *path, pkcs12_t *p12)
{
	enumerator_t *enumerator;
	certificate_t *cert;
	private_key_t *private;
	chunk_t encoding;
	bool loaded = TRUE;

	enumerator = p12->create_cert_enumerator(p12);
	while (loaded && enumerator->enumerate(enumerator, &cert))
	{
		loaded = FALSE;
		if (cert->get_encoding(cert, CERT_ASN1_DER, &encoding))
		{
			loaded = load_cert(ctx, path, CERT_X509, X509_NONE, encoding);
			if (loaded)
			{
				fprintf(stderr, "  %Y\n", cert->get_subject(cert));
			}
			free(encoding.ptr);
		}
		else
		{
			fprintf(stderr, "encoding certificate from '%s' failed\n", path);
		}
	}
	enumerator->destroy(enumerator);

	enumerator = p12->create_key_enumerator(p12);
	while (loaded && enumerator->enumerate(enumerator, &private))
	{
		loaded = load_key_anytype(ctx, path, private);
	}
	enumerator->destroy(enumerator);

	return loaded;
}

/**
 * Try to decrypt and load credentials from a container
 */
static bool load_encrypted_container(load_ctx_t *ctx, char *rel, char *path,
									 char *type, chunk_t data)
{
	container_t *container;
	bool loaded = FALSE;

	container = decrypt_with_config(ctx, rel, type, data);
	if (!container && !ctx->noprompt)
	{
		container = decrypt(rel, type, data);
	}
	if (container)
	{
		switch (container->get_type(container))
		{
			case CONTAINER_PKCS12:
				loaded = load_pkcs12(ctx, path, (pkcs12_t*)container);
				break;
			default:
				break;
		}
		container->destroy(container);
	}
	return loaded;
}

/**
 * Load credential containers from a directory
 */
static void load_containers(load_ctx_t *ctx, char *type, char *dir)
{
	enumerator_t *enumerator;
	struct stat st;
	chunk_t *map;
	char *path, *rel, buf[PATH_MAX];

	snprintf(buf, sizeof(buf), "%s%s%s", swanctl_dir, DIRECTORY_SEPARATOR, dir);
	dir = buf;

	enumerator = enumerator_create_directory(dir);
	if (enumerator)
	{
		while (enumerator->enumerate(enumerator, &rel, &path, &st))
		{
			if (S_ISREG(st.st_mode))
			{
				map = chunk_map(path, FALSE);
				if (map)
				{
					load_encrypted_container(ctx, rel, path, type, *map);
					chunk_unmap(map);
				}
				else
				{
					fprintf(stderr, "mapping '%s' failed: %s, skipped\n",
							path, strerror(errno));
				}
			}
		}
		enumerator->destroy(enumerator);
	}
}

/**
 * Load a single private key on a token over vici
 */
static bool load_token(load_ctx_t *ctx, char *name, char *pin)
{
	vici_req_t *req;
	vici_res_t *res;
	enumerator_t *enumerator;
	char *key, *value, *id;
	bool ret = TRUE;

	req = vici_begin("load-token");

	enumerator = ctx->cfg->create_key_value_enumerator(ctx->cfg, "secrets.%s",
													   name);
	while (enumerator->enumerate(enumerator, &key, &value))
	{
		vici_add_key_valuef(req, key, "%s", value);
	}
	enumerator->destroy(enumerator);

	if (pin)
	{
		vici_add_key_valuef(req, "pin", "%s", pin);
	}
	res = vici_submit(req, ctx->conn);
	if (!res)
	{
		fprintf(stderr, "load-token request failed: %s\n", strerror(errno));
		return FALSE;
	}
	if (ctx->format & COMMAND_FORMAT_RAW)
	{
		vici_dump(res, "load-token reply", ctx->format & COMMAND_FORMAT_PRETTY,
				  stdout);
	}
	else if (!streq(vici_find_str(res, "no", "success"), "yes"))
	{
		fprintf(stderr, "loading '%s' failed: %s\n",
				name, vici_find_str(res, "", "errmsg"));
		ret = FALSE;
	}
	else
	{
		id = vici_find_str(res, "", "id");
		printf("loaded key %s from token [keyid: %s]\n", name, id);
		free(ctx->keys->remove(ctx->keys, id));
	}
	vici_free_res(res);
	return ret;
}

/**
 * Load keys from tokens
 */
static void load_tokens(load_ctx_t *ctx)
{
	enumerator_t *enumerator;
	char *section, *pin = NULL, prompt[128];

	enumerator = ctx->cfg->create_section_enumerator(ctx->cfg, "secrets");
	while (enumerator->enumerate(enumerator, &section))
	{
		if (strpfx(section, "token"))
		{
			if (!ctx->noprompt &&
				!ctx->cfg->get_str(ctx->cfg, "secrets.%s.pin", NULL, section))
			{
#ifdef HAVE_GETPASS
				snprintf(prompt, sizeof(prompt), "PIN for %s: ", section);
				pin = strdupnull(getpass(prompt));
#endif
			}
			load_token(ctx, section, pin);
			if (pin)
			{
				memwipe(pin, strlen(pin));
				free(pin);
				pin = NULL;
			}
		}
	}
	enumerator->destroy(enumerator);
}



/**
 * Load a single secret over VICI
 */
static bool load_secret(load_ctx_t *ctx, char *section)
{
	enumerator_t *enumerator;
	vici_req_t *req;
	vici_res_t *res;
	chunk_t data;
	char *key, *value, *type = NULL;
	bool ret = TRUE;
	int i;
	char *types[] = {
		"eap",
		"xauth",
		"ntlm",
		"ike",
		"ppk",
		"private",
		"rsa",
		"ecdsa",
		"bliss",
		"pkcs8",
		"pkcs12",
		"token",
	};

	for (i = 0; i < countof(types); i++)
	{
		if (strpfx(section, types[i]))
		{
			type = types[i];
			break;
		}
	}
	if (!type)
	{
		fprintf(stderr, "ignoring unsupported secret '%s'\n", section);
		return FALSE;
	}
	if (!streq(type, "eap") && !streq(type, "xauth") && !streq(type, "ntlm") &&
		!streq(type, "ike") && !streq(type, "ppk"))
	{	/* skip non-shared secrets */
		return TRUE;
	}

	value = ctx->cfg->get_str(ctx->cfg, "secrets.%s.secret", NULL, section);
	if (!value)
	{
		fprintf(stderr, "missing secret in '%s', ignored\n", section);
		return FALSE;
	}
	if (strcasepfx(value, "0x"))
	{
		data = chunk_from_hex(chunk_from_str(value + 2), NULL);
	}
	else if (strcasepfx(value, "0s"))
	{
		data = chunk_from_base64(chunk_from_str(value + 2), NULL);
	}
	else
	{
		data = chunk_clone(chunk_from_str(value));
	}

	req = vici_begin("load-shared");

	vici_add_key_valuef(req, "id", "%s", section);
	vici_add_key_valuef(req, "type", "%s", type);
	vici_add_key_value(req, "data", data.ptr, data.len);
	chunk_clear(&data);

	vici_begin_list(req, "owners");
	enumerator = ctx->cfg->create_key_value_enumerator(ctx->cfg, "secrets.%s",
													   section);
	while (enumerator->enumerate(enumerator, &key, &value))
	{
		if (strpfx(key, "id"))
		{
			vici_add_list_itemf(req, "%s", value);
		}
	}
	enumerator->destroy(enumerator);
	vici_end_list(req);

	res = vici_submit(req, ctx->conn);
	if (!res)
	{
		fprintf(stderr, "load-shared request failed: %s\n", strerror(errno));
		return FALSE;
	}
	if (ctx->format & COMMAND_FORMAT_RAW)
	{
		vici_dump(res, "load-shared reply", ctx->format & COMMAND_FORMAT_PRETTY,
				  stdout);
	}
	else if (!streq(vici_find_str(res, "no", "success"), "yes"))
	{
		fprintf(stderr, "loading shared secret failed: %s\n",
				vici_find_str(res, "", "errmsg"));
		ret = FALSE;
	}
	else
	{
		printf("loaded %s secret '%s'\n", type, section);
	}
	if (ret)
	{
		free(ctx->shared->remove(ctx->shared, section));
	}
	vici_free_res(res);
	return ret;
}

CALLBACK(get_id, int,
	hashtable_t *ht, vici_res_t *res, char *name, void *value, int len)
{
	if (streq(name, "keys"))
	{
		char *str;

		if (asprintf(&str, "%.*s", len, value) != -1)
		{
			free(ht->put(ht, str, str));
		}
	}
	return 0;
}

/**
 * Get a list of currently loaded private and shared keys
 */
static void get_creds(load_ctx_t *ctx)
{
	vici_res_t *res;

	res = vici_submit(vici_begin("get-keys"), ctx->conn);
	if (res)
	{
		if (ctx->format & COMMAND_FORMAT_RAW)
		{
			vici_dump(res, "get-keys reply", ctx->format & COMMAND_FORMAT_PRETTY,
					  stdout);
		}
		vici_parse_cb(res, NULL, NULL, get_id, ctx->keys);
		vici_free_res(res);
	}
	res = vici_submit(vici_begin("get-shared"), ctx->conn);
	if (res)
	{
		if (ctx->format & COMMAND_FORMAT_RAW)
		{
			vici_dump(res, "get-shared reply", ctx->format & COMMAND_FORMAT_PRETTY,
					  stdout);
		}
		vici_parse_cb(res, NULL, NULL, get_id, ctx->shared);
		vici_free_res(res);
	}
}

/**
 * Remove a given key
 */
static bool unload_key(load_ctx_t *ctx, char *command, char *id)
{
	vici_req_t *req;
	vici_res_t *res;
	char buf[BUF_LEN];
	bool ret = TRUE;

	req = vici_begin(command);

	vici_add_key_valuef(req, "id", "%s", id);

	res = vici_submit(req, ctx->conn);
	if (!res)
	{
		fprintf(stderr, "%s request failed: %s\n", command, strerror(errno));
		return FALSE;
	}
	if (ctx->format & COMMAND_FORMAT_RAW)
	{
		snprintf(buf, sizeof(buf), "%s reply", command);
		vici_dump(res, buf, ctx->format & COMMAND_FORMAT_PRETTY, stdout);
	}
	else if (!streq(vici_find_str(res, "no", "success"), "yes"))
	{
		fprintf(stderr, "unloading key '%s' failed: %s\n",
				id, vici_find_str(res, "", "errmsg"));
		ret = FALSE;
	}
	vici_free_res(res);
	return ret;
}

/**
 * Remove all keys in the given hashtable using the given command
 */
static void unload_keys(load_ctx_t *ctx, hashtable_t *ht, char *command)
{
	enumerator_t *enumerator;
	char *id;

	enumerator = ht->create_enumerator(ht);
	while (enumerator->enumerate(enumerator, &id, NULL))
	{
		unload_key(ctx, command, id);
	}
	enumerator->destroy(enumerator);
}

/**
 * Clear all currently loaded credentials
 */
static bool clear_creds(vici_conn_t *conn, command_format_options_t format)
{
	vici_res_t *res;

	res = vici_submit(vici_begin("clear-creds"), conn);
	if (!res)
	{
		fprintf(stderr, "clear-creds request failed: %s\n", strerror(errno));
		return FALSE;
	}
	if (format & COMMAND_FORMAT_RAW)
	{
		vici_dump(res, "clear-creds reply", format & COMMAND_FORMAT_PRETTY,
				  stdout);
	}
	vici_free_res(res);
	return TRUE;
}

/**
 * See header.
 */
int load_creds_cfg(vici_conn_t *conn, command_format_options_t format,
				   settings_t *cfg, bool clear, bool noprompt)
{
	enumerator_t *enumerator;
	char *section;
	load_ctx_t ctx = {
		.conn = conn,
		.format = format,
		.noprompt = noprompt,
		.cfg = cfg,
		.keys = hashtable_create(hashtable_hash_str, hashtable_equals_str, 8),
		.shared = hashtable_create(hashtable_hash_str, hashtable_equals_str, 8),
	};

	if (clear)
	{
		if (!clear_creds(conn, format))
		{
			return ECONNREFUSED;
		}
	}

	get_creds(&ctx);

	load_certs(&ctx, "x509",     SWANCTL_X509DIR);
	load_certs(&ctx, "x509ca",   SWANCTL_X509CADIR);
	load_certs(&ctx, "x509ocsp", SWANCTL_X509OCSPDIR);
	load_certs(&ctx, "x509aa",   SWANCTL_X509AADIR);
	load_certs(&ctx, "x509ac",   SWANCTL_X509ACDIR);
	load_certs(&ctx, "x509crl",  SWANCTL_X509CRLDIR);
	load_certs(&ctx, "pubkey",   SWANCTL_PUBKEYDIR);

	load_keys(&ctx, "private", SWANCTL_PRIVATEDIR);
	load_keys(&ctx, "rsa",     SWANCTL_RSADIR);
	load_keys(&ctx, "ecdsa",   SWANCTL_ECDSADIR);
	load_keys(&ctx, "bliss",   SWANCTL_BLISSDIR);
	load_keys(&ctx, "pkcs8",   SWANCTL_PKCS8DIR);

	load_containers(&ctx, "pkcs12", SWANCTL_PKCS12DIR);

	load_tokens(&ctx);

	enumerator = cfg->create_section_enumerator(cfg, "secrets");
	while (enumerator->enumerate(enumerator, &section))
	{
		load_secret(&ctx, section);
	}
	enumerator->destroy(enumerator);

	unload_keys(&ctx, ctx.keys, "unload-key");
	unload_keys(&ctx, ctx.shared, "unload-shared");

	ctx.keys->destroy_function(ctx.keys, (void*)free);
	ctx.shared->destroy_function(ctx.shared, (void*)free);
	return 0;
}

static int load_creds(vici_conn_t *conn)
{
	bool clear = FALSE, noprompt = FALSE;
	command_format_options_t format = COMMAND_FORMAT_NONE;
	settings_t *cfg;
	char *arg, *file = NULL;
	int ret;

	while (TRUE)
	{
		switch (command_getopt(&arg))
		{
			case 'h':
				return command_usage(NULL);
			case 'c':
				clear = TRUE;
				continue;
			case 'n':
				noprompt = TRUE;
				continue;
			case 'P':
				format |= COMMAND_FORMAT_PRETTY;
				/* fall through to raw */
			case 'r':
				format |= COMMAND_FORMAT_RAW;
				continue;
			case 'f':
				file = arg;
				continue;
			case EOF:
				break;
			default:
				return command_usage("invalid --load-creds option");
		}
		break;
	}

	cfg = load_swanctl_conf(file);
	if (!cfg)
	{
		return EINVAL;
	}

	ret = load_creds_cfg(conn, format, cfg, clear, noprompt);

	cfg->destroy(cfg);

	return ret;
}

/**
 * Register the command.
 */
static void __attribute__ ((constructor))reg()
{
	command_register((command_t) {
		load_creds, 's', "load-creds", "(re-)load credentials",
		{"[--raw|--pretty] [--clear] [--noprompt]"},
		{
			{"help",		'h', 0, "show usage information"},
			{"clear",		'c', 0, "clear previously loaded credentials"},
			{"noprompt",	'n', 0, "do not prompt for passwords"},
			{"raw",			'r', 0, "dump raw response message"},
			{"pretty",		'P', 0, "dump raw response message in pretty print"},
			{"file",		'f', 1, "custom path to swanctl.conf"},
		}
	});
}
