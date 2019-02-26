/*
 * Copyright (C) 2018 Tobias Brunner
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
 * Copyright (C) 2016 Codrut Cristian Grosu (codrut.cristian.grosu@gmail.com)
 * Copyright (C) 2016 IXIA (http://www.ixiacom.com)
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

#define _GNU_SOURCE

#include "save_keys_listener.h"

#include <stdio.h>
#include <inttypes.h>
#include <errno.h>

#include <daemon.h>

typedef struct private_save_keys_listener_t private_save_keys_listener_t;
typedef struct algo_map_t algo_map_t;

/**
 * Name for IKEv1 decryption table file
 */
static char *ikev1_name = "ikev1_decryption_table";

/**
 * Name for IKEv2 decryption table file
 */
static char *ikev2_name = "ikev2_decryption_table";

/**
 * Name for esp decryption table file
 */
static char *esp_name = "esp_sa";

/**
 * Private data.
 */
struct private_save_keys_listener_t {

	/**
	 * Public interface.
	 */
	save_keys_listener_t public;

	/**
	 * Path to the directory where the decryption tables will be stored.
	 */
	char *path;

	/**
	 * Whether to save IKE keys
	 */
	bool ike;

	/**
	 * Whether to save ESP keys
	 */
	bool esp;
};

METHOD(save_keys_listener_t, destroy, void,
	private_save_keys_listener_t *this)
{
	free(this);
}

/**
 * Mapping strongSwan identifiers to Wireshark names
 */
struct algo_map_t {

	/**
	 * IKE identifier
	 */
	const uint16_t ike;

	/**
	 * Optional key length
	 */
	const int key_len;

	/**
	 * Name of the algorithm in wireshark
	 */
	const char *name;
};

/**
 * Map an algorithm identifier to a name
 */
static inline const char *algo_name(algo_map_t *map, int count,
									uint16_t alg, int key_len)
{
	int i;

	for (i = 0; i < count; i++)
	{
		if (map[i].ike == alg)
		{
			if (map[i].key_len == -1 || map[i].key_len == key_len)
			{
				return map[i].name;
			}
		}
	}
	return NULL;
}

/**
 * Wireshark IKE algorithm identifiers for encryption
 */
static algo_map_t ike_encr[] = {
	{ ENCR_3DES,           -1, "3DES [RFC2451]"                          },
	{ ENCR_NULL,           -1, "NULL [RFC2410]"                          },
	{ ENCR_AES_CBC,       128, "AES-CBC-128 [RFC3602]"                   },
	{ ENCR_AES_CBC,       192, "AES-CBC-192 [RFC3602]"                   },
	{ ENCR_AES_CBC,       256, "AES-CBC-256 [RFC3602]"                   },
	{ ENCR_AES_CTR,       128, "AES-CTR-128 [RFC5930]"                   },
	{ ENCR_AES_CTR,       192, "AES-CTR-192 [RFC5930]"                   },
	{ ENCR_AES_CTR,       256, "AES-CTR-256 [RFC5930]"                   },
	{ ENCR_AES_GCM_ICV8,  128, "AES-GCM-128 with 8 octet ICV [RFC5282]"  },
	{ ENCR_AES_GCM_ICV8,  192, "AES-GCM-192 with 8 octet ICV [RFC5282]"  },
	{ ENCR_AES_GCM_ICV8,  256, "AES-GCM-256 with 8 octet ICV [RFC5282]"  },
	{ ENCR_AES_GCM_ICV12, 128, "AES-GCM-128 with 12 octet ICV [RFC5282]" },
	{ ENCR_AES_GCM_ICV12, 192, "AES-GCM-192 with 12 octet ICV [RFC5282]" },
	{ ENCR_AES_GCM_ICV12, 256, "AES-GCM-256 with 12 octet ICV [RFC5282]" },
	{ ENCR_AES_GCM_ICV16, 128, "AES-GCM-128 with 16 octet ICV [RFC5282]" },
	{ ENCR_AES_GCM_ICV16, 192, "AES-GCM-192 with 16 octet ICV [RFC5282]" },
	{ ENCR_AES_GCM_ICV16, 256, "AES-GCM-256 with 16 octet ICV [RFC5282]" },
	{ ENCR_AES_CCM_ICV8,  128, "AES-CCM-128 with 8 octet ICV [RFC5282]"  },
	{ ENCR_AES_CCM_ICV8,  192, "AES-CCM-192 with 8 octet ICV [RFC5282]"  },
	{ ENCR_AES_CCM_ICV8,  256, "AES-CCM-256 with 8 octet ICV [RFC5282]"  },
	{ ENCR_AES_CCM_ICV12, 128, "AES-CCM-128 with 12 octet ICV [RFC5282]" },
	{ ENCR_AES_CCM_ICV12, 192, "AES-CCM-192 with 12 octet ICV [RFC5282]" },
	{ ENCR_AES_CCM_ICV12, 256, "AES-CCM-256 with 12 octet ICV [RFC5282]" },
	{ ENCR_AES_CCM_ICV16, 128, "AES-CCM-128 with 16 octet ICV [RFC5282]" },
	{ ENCR_AES_CCM_ICV16, 192, "AES-CCM-192 with 16 octet ICV [RFC5282]" },
	{ ENCR_AES_CCM_ICV16, 256, "AES-CCM-256 with 16 octet ICV [RFC5282]" },
};

/**
 * Wireshark IKE algorithms for integrity
 */
static algo_map_t ike_integ[] = {
	{ AUTH_HMAC_MD5_96,       -1, "HMAC_MD5_96 [RFC2403]"       },
	{ AUTH_HMAC_SHA1_96,      -1, "HMAC_SHA1_96 [RFC2404]"      },
	{ AUTH_HMAC_MD5_128,      -1, "HMAC_MD5_128 [RFC4595]"      },
	{ AUTH_HMAC_SHA1_160,     -1, "HMAC_SHA1_160 [RFC4595]"     },
	{ AUTH_HMAC_SHA2_256_128, -1, "HMAC_SHA2_256_128 [RFC4868]" },
	{ AUTH_HMAC_SHA2_384_192, -1, "HMAC_SHA2_384_192 [RFC4868]" },
	{ AUTH_HMAC_SHA2_512_256, -1, "HMAC_SHA2_512_256 [RFC4868]" },
	{ AUTH_HMAC_SHA2_256_96,  -1, "HMAC_SHA2_256_96 [draft-ietf-ipsec-ciph-sha-256-00]" },
	{ AUTH_UNDEFINED,         -1, "NONE [RFC4306]"              },
};

/**
 * Map an IKE proposal
 */
static inline void ike_names(proposal_t *proposal, const char **enc,
							 const char **integ)
{
	uint16_t alg, len;

	if (proposal->get_algorithm(proposal, ENCRYPTION_ALGORITHM, &alg, &len))
	{
		*enc = algo_name(ike_encr, countof(ike_encr), alg, len);
	}
	if (encryption_algorithm_is_aead(alg))
	{
		alg = AUTH_UNDEFINED;
	}
	else if (!proposal->get_algorithm(proposal, INTEGRITY_ALGORITHM, &alg, NULL))
	{
		return;
	}
	*integ = algo_name(ike_integ, countof(ike_integ), alg, -1);
}

/**
 * Wireshark ESP algorithm identifiers for encryption
 */
static algo_map_t esp_encr[] = {
	{ ENCR_NULL,          -1, "NULL"                    },
	{ ENCR_3DES,          -1, "TripleDes-CBC [RFC2451]" },
	{ ENCR_AES_CBC,       -1, "AES-CBC [RFC3602]"       },
	{ ENCR_AES_CTR,       -1, "AES-CTR [RFC3686]"       },
	{ ENCR_DES,           -1, "DES-CBC [RFC2405]"       },
	{ ENCR_CAST,          -1, "CAST5-CBC [RFC2144]"     },
	{ ENCR_BLOWFISH,      -1, "BLOWFISH-CBC [RFC2451]"  },
	{ ENCR_TWOFISH_CBC,   -1, "TWOFISH-CBC"             },
	{ ENCR_AES_GCM_ICV8,  -1, "AES-GCM [RFC4106]"       },
	{ ENCR_AES_GCM_ICV12, -1, "AES-GCM [RFC4106]"       },
	{ ENCR_AES_GCM_ICV16, -1, "AES-GCM [RFC4106]"       },
};

/**
 * Wireshark ESP algorithms for integrity
 */
static algo_map_t esp_integ[] = {
	{ AUTH_HMAC_SHA1_96,       -1, "HMAC-SHA-1-96 [RFC2404]"                  },
	{ AUTH_HMAC_MD5_96,        -1, "HMAC-MD5-96 [RFC2403]"                    },
	{ AUTH_HMAC_SHA2_256_128,  -1, "HMAC-SHA-256-128 [RFC4868]"               },
	{ AUTH_HMAC_SHA2_384_192,  -1, "HMAC-SHA-384-192 [RFC4868]"               },
	{ AUTH_HMAC_SHA2_512_256,  -1, "HMAC-SHA-512-256 [RFC4868]"               },
	{ AUTH_HMAC_SHA2_256_96,   -1, "HMAC-SHA-256-96 [draft-ietf-ipsec-ciph-sha-256-00]" },
	{ AUTH_UNDEFINED,          64, "ANY 64 bit authentication [no checking]"  },
	{ AUTH_UNDEFINED,          96, "ANY 96 bit authentication [no checking]"  },
	{ AUTH_UNDEFINED,         128, "ANY 128 bit authentication [no checking]" },
	{ AUTH_UNDEFINED,         192, "ANY 192 bit authentication [no checking]" },
	{ AUTH_UNDEFINED,         256, "ANY 256 bit authentication [no checking]" },
	{ AUTH_UNDEFINED,          -1, "NULL"                                     },
};

/**
 * Map an ESP proposal
 */
static inline void esp_names(proposal_t *proposal, const char **enc,
							 const char **integ)
{
	uint16_t alg, len;

	if (proposal->get_algorithm(proposal, ENCRYPTION_ALGORITHM, &alg, &len))
	{
		*enc = algo_name(esp_encr, countof(esp_encr), alg, len);
	}
	len = -1;
	if (!proposal->get_algorithm(proposal, INTEGRITY_ALGORITHM, &alg, NULL))
	{
		switch (alg)
		{
			case ENCR_AES_GCM_ICV8:
				len = 64;
				break;
			case ENCR_AES_GCM_ICV12:
				len = 64;
				break;
			case ENCR_AES_GCM_ICV16:
				len = 128;
				break;
		}
		alg = AUTH_UNDEFINED;
	}
	*integ = algo_name(esp_integ, countof(esp_integ), alg, len);
}

METHOD(listener_t, ike_derived_keys, bool,
	private_save_keys_listener_t *this, ike_sa_t *ike_sa, chunk_t sk_ei,
	chunk_t sk_er, chunk_t sk_ai, chunk_t sk_ar)
{
	ike_version_t version;
	ike_sa_id_t *id;
	const char *enc = NULL, *integ = NULL;
	char *path, *name;
	FILE *file;

	if (!this->path || !this->ike)
	{
		return TRUE;
	}

	version = ike_sa->get_version(ike_sa);
	name = version == IKEV2 ? ikev2_name : ikev1_name;
	if (asprintf(&path, "%s/%s", this->path, name) < 0)
	{
		DBG1(DBG_IKE, "failed to build path to IKE key table");
		return TRUE;
	}

	file = fopen(path, "a");
	if (file)
	{
		id = ike_sa->get_id(ike_sa);
		if (version == IKEV2)
		{
			ike_names(ike_sa->get_proposal(ike_sa), &enc, &integ);
			if (enc && integ)
			{
				fprintf(file, "%.16"PRIx64",%.16"PRIx64",%+B,%+B,\"%s\","
						"%+B,%+B,\"%s\"\n", be64toh(id->get_initiator_spi(id)),
						be64toh(id->get_responder_spi(id)), &sk_ei, &sk_er,
						enc, &sk_ai, &sk_ar, integ);
			}
		}
		else
		{
			fprintf(file, "%.16"PRIx64",%+B\n",
					be64toh(id->get_initiator_spi(id)), &sk_ei);
		}
		fclose(file);
	}
	else
	{
		DBG1(DBG_IKE, "failed to open IKE key table '%s': %s", path,
			 strerror(errno));
	}
	free(path);
	return TRUE;
}

METHOD(listener_t, child_derived_keys, bool,
	private_save_keys_listener_t *this, ike_sa_t *ike_sa, child_sa_t *child_sa,
	bool initiator, chunk_t encr_i, chunk_t encr_r, chunk_t integ_i,
	chunk_t integ_r)
{
	host_t *init, *resp;
	uint32_t spi_i, spi_r;
	const char *enc = NULL, *integ = NULL;
	char *path, *family;
	FILE *file;

	if (!this->path || !this->esp ||
		child_sa->get_protocol(child_sa) != PROTO_ESP)
	{
		return TRUE;
	}

	if (asprintf(&path, "%s/%s", this->path, esp_name) < 0)
	{
		DBG1(DBG_CHD, "failed to build path to ESP key table");
		return TRUE;
	}

	file = fopen(path, "a");
	if (file)
	{
		esp_names(child_sa->get_proposal(child_sa), &enc, &integ);
		if (enc && integ)
		{
			/* Since the IPs are printed this is not compatible with MOBIKE */
			if (initiator)
			{
				init = ike_sa->get_my_host(ike_sa);
				resp = ike_sa->get_other_host(ike_sa);
			}
			else
			{
				init = ike_sa->get_other_host(ike_sa);
				resp = ike_sa->get_my_host(ike_sa);
			}
			spi_i = child_sa->get_spi(child_sa, initiator);
			spi_r = child_sa->get_spi(child_sa, !initiator);
			family = init->get_family(init) == AF_INET ? "IPv4" : "IPv6";
			fprintf(file, "\"%s\",\"%H\",\"%H\",\"0x%.8x\",\"%s\",\"0x%+B\","
					"\"%s\",\"0x%+B\"\n", family, init, resp, ntohl(spi_r), enc,
					&encr_i, integ, &integ_i);
			fprintf(file, "\"%s\",\"%H\",\"%H\",\"0x%.8x\",\"%s\",\"0x%+B\","
					"\"%s\",\"0x%+B\"\n", family, resp, init, ntohl(spi_i), enc,
					&encr_r, integ, &integ_r);
		}
		fclose(file);
	}
	else
	{
		DBG1(DBG_CHD, "failed to open ESP key table '%s': %s", path,
			 strerror(errno));
	}
	free(path);
	return TRUE;
}

/**
 * See header.
 */
save_keys_listener_t *save_keys_listener_create()
{
	private_save_keys_listener_t *this;

	INIT(this,
		.public = {
			.listener = {
				.ike_derived_keys = _ike_derived_keys,
				.child_derived_keys = _child_derived_keys,
			},
			.destroy = _destroy,
		},
		.path = lib->settings->get_str(lib->settings,
									   "%s.plugins.save-keys.wireshark_keys",
									   NULL, lib->ns),
		.esp = lib->settings->get_bool(lib->settings,
									   "%s.plugins.save-keys.esp",
									   FALSE, lib->ns),
		.ike = lib->settings->get_bool(lib->settings,
									   "%s.plugins.save-keys.ike",
									   FALSE, lib->ns),
	);

	if (this->path && (this->ike || this->esp))
	{
		char *keys = "IKE";

		if (this->ike && this->esp)
		{
			keys = "IKE AND ESP";
		}
		else if (this->esp)
		{
			keys = "ESP";
		}
		DBG0(DBG_DMN, "!!", keys, this->path);
		DBG0(DBG_DMN, "!! WARNING: SAVING %s KEYS TO '%s'", keys, this->path);
		DBG0(DBG_DMN, "!!", keys, this->path);
	}
	return &this->public;
}
