/*
 * Copyright (C) 2013 Tobias Brunner
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <errno.h>

#include <library.h>

/** plugins to load */
#undef PLUGINS
#define PLUGINS "openssl"

/**
 * Context
 */
static struct {
	/** input file */
	FILE *in;
	/** output file */
	FILE *out;
	/** whether to use GCM or CBC */
	bool use_gcm;
	/** whether to run the Monte Carlo Test */
	bool use_mct;
	/** whether to test encryption or decryption */
	bool decrypt;
	/** IV length in bits in case of GCM */
	int ivlen;
	/** ICV length in bits in case of GCM */
	int icvlen;
} ctx;

/**
 * Types of parameters of a test vector
 */
typedef enum {
	PARAM_UNKNOWN,
	PARAM_COUNT,
	PARAM_KEY,
	PARAM_IV,
	PARAM_PLAINTEXT,
	PARAM_CIPHERTEXT,
	PARAM_AAD,
	PARAM_ICV,
} param_t;

static param_t parse_parameter(char *param)
{
	if (strcaseeq(param, "COUNT"))
	{
		return PARAM_COUNT;
	}
	if (strcaseeq(param, "KEY"))
	{
		return PARAM_KEY;
	}
	if (strcaseeq(param, "IV"))
	{
		return PARAM_IV;
	}
	if (strcaseeq(param, "PLAINTEXT") ||
		strcaseeq(param, "PT"))
	{
		return PARAM_PLAINTEXT;
	}
	if (strcaseeq(param, "CIPHERTEXT") ||
		strcaseeq(param, "CT"))
	{
		return PARAM_CIPHERTEXT;
	}
	if (strcaseeq(param, "AAD"))
	{
		return PARAM_AAD;
	}
	if (strcaseeq(param, "TAG"))
	{
		return PARAM_ICV;
	}
	return PARAM_UNKNOWN;
}

/**
 * Test vector
 */
typedef struct {
	/** encryption/decryption key */
	chunk_t key;
	/** initialization vector */
	chunk_t iv;
	/** plain text */
	chunk_t plain;
	/** cipher text */
	chunk_t cipher;
	/** associated data */
	chunk_t aad;
	/** ICV/tag */
	chunk_t icv;
	/** whether the IV was provided */
	bool external_iv;
	/** whether the decryption/verification in GCM mode was successful */
	bool success;
} test_vector_t;

static void test_vector_free(test_vector_t *test)
{
	chunk_free(&test->key);
	chunk_free(&test->iv);
	chunk_free(&test->plain);
	chunk_free(&test->cipher);
	chunk_free(&test->aad);
	chunk_free(&test->icv);
}

static void print_result(test_vector_t *test)
{
	if (ctx.use_gcm)
	{
		if (ctx.decrypt)
		{
			if (test->success)
			{
				fprintf(ctx.out, "PT = %+B\n", &test->plain);
			}
			else
			{
				fprintf(ctx.out, "FAIL\n");
			}
			return;
		}
		if (!test->external_iv)
		{
			fprintf(ctx.out, "IV = %+B\n", &test->iv);
		}
		fprintf(ctx.out, "CT = %+B\n", &test->cipher);
		fprintf(ctx.out, "Tag = %+B\n", &test->icv);
	}
	else
	{
		fprintf(ctx.out, "%s = %+B\n", ctx.decrypt ? "PLAINTEXT" : "CIPHERTEXT",
				ctx.decrypt ? &test->plain : &test->cipher);
	}
}

static bool get_next_test_vector(test_vector_t *test)
{
	param_t param = PARAM_UNKNOWN;
	char line[512];

	memset(test, 0, sizeof(test_vector_t));

	while (fgets(line, sizeof(line), ctx.in))
	{
		enumerator_t *enumerator;
		chunk_t value = chunk_empty;
		char *token;
		int i;

		switch (line[0])
		{
			case '\n':
			case '\r':
			case '#':
			case '\0':
				/* copy comments, empty lines etc. directly to the output */
				if (param != PARAM_UNKNOWN)
				{	/* seems we got a complete test vector */
					return TRUE;
				}
				fputs(line, ctx.out);
				continue;
			case '[':
				/* control directives */
				fputs(line, ctx.out);
				if (strpfx(line, "[ENCRYPT]"))
				{
					ctx.decrypt = FALSE;
				}
				else if (strpfx(line, "[DECRYPT]"))
				{
					ctx.decrypt = TRUE;
				}
				else if (strcasepfx(line, "[IVlen = "))
				{
					ctx.ivlen = atoi(line + strlen("[IVlen = "));
				}
				else if (strcasepfx(line, "[Taglen = "))
				{
					ctx.icvlen = atoi(line + strlen("[Taglen = "));
				}
				continue;
			default:
				/* we assume the rest of the lines are PARAM = VALUE pairs*/
				fputs(line, ctx.out);
				break;
		}

		i = 0;
		enumerator = enumerator_create_token(line, "=", " \n\r");
		while (enumerator->enumerate(enumerator, &token))
		{
			switch (i++)
			{
				case 0: /* PARAM */
					param = parse_parameter(token);
					continue;
				case 1: /* VALUE */
					if (param != PARAM_UNKNOWN && param != PARAM_COUNT)
					{
						value = chunk_from_hex(chunk_from_str(token), NULL);
					}
					else
					{
						value = chunk_empty;
					}
					continue;
				default:
					break;
			}
			break;
		}
		enumerator->destroy(enumerator);
		if (i < 2)
		{
			value = chunk_empty;
		}
		switch (param)
		{
			case PARAM_KEY:
				test->key = value;
				break;
			case PARAM_IV:
				test->iv = value;
				test->external_iv = TRUE;
				break;
			case PARAM_PLAINTEXT:
				test->plain = value;
				break;
			case PARAM_CIPHERTEXT:
				test->cipher = value;
				break;
			case PARAM_AAD:
				test->aad = value;
				break;
			case PARAM_ICV:
				test->icv = value;
				break;
			default:
				chunk_free(&value);
				break;
		}
	}
	if (param != PARAM_UNKNOWN)
	{	/* could be that the file ended with a complete test vector */
		return TRUE;
	}
	return FALSE;
}

static bool verify_test_vector(test_vector_t *test)
{
	if (ctx.use_gcm)
	{
		if (ctx.decrypt)
		{
			return test->key.ptr && test->iv.ptr && test->cipher.ptr &&
				   test->icv.ptr;
		}
		return test->key.ptr && test->plain.ptr;
	}
	if (ctx.decrypt)
	{
		return test->key.ptr && test->iv.ptr && test->cipher.ptr;
	}
	return test->key.ptr && test->iv.ptr && test->plain.ptr;
}

static bool do_test_gcm(test_vector_t *test)
{
	encryption_algorithm_t alg;
	chunk_t key, iv;
	aead_t *aead;
	size_t saltlen, ivlen;

	switch (ctx.icvlen / 8)
	{
		case 8:
			alg = ENCR_AES_GCM_ICV8;
			break;
		case 12:
			alg = ENCR_AES_GCM_ICV12;
			break;
		case 16:
			alg = ENCR_AES_GCM_ICV16;
			break;
		default:
			DBG1(DBG_APP, "unsupported ICV length: %d", ctx.icvlen);
			return FALSE;
	}

	aead = lib->crypto->create_aead(lib->crypto, alg, test->key.len, 4);
	if (!aead)
	{
		DBG1(DBG_APP, "algorithm %N or key length (%d bits) not supported",
			 encryption_algorithm_names, alg, test->key.len * 8);
		return FALSE;
	}
	/* our API is quite RFC 4106 specific, that is, part of the IV is provided
	 * at the end of the key. */
	saltlen = aead->get_key_size(aead) - test->key.len;
	ivlen = aead->get_iv_size(aead);
	if (ctx.ivlen / 8 != saltlen + ivlen)
	{
		DBG1(DBG_APP, "unsupported IV length: %d", ctx.ivlen);
		aead->destroy(aead);
		return FALSE;
	}
	if (!test->external_iv)
	{
		rng_t *rng;

		/* the IV consists of saltlen random bytes (usually additional keymat)
		 * followed by a counter, zero here */
		test->iv = chunk_alloc(saltlen + ivlen);
		memset(test->iv.ptr, 0, test->iv.len);
		rng = lib->crypto->create_rng(lib->crypto, RNG_STRONG);
		if (!rng || !rng->get_bytes(rng, saltlen, test->iv.ptr))
		{
			DBG1(DBG_APP, "failed to generate IV");
			DESTROY_IF(rng);
			aead->destroy(aead);
			return FALSE;
		}
		rng->destroy(rng);
	}
	key = chunk_alloca(test->key.len + saltlen);
	memcpy(key.ptr, test->key.ptr, test->key.len);
	memcpy(key.ptr + test->key.len, test->iv.ptr, saltlen);
	iv = chunk_alloca(ivlen);
	memcpy(iv.ptr, test->iv.ptr + saltlen, iv.len);
	if (!aead->set_key(aead, key))
	{
		DBG1(DBG_APP, "failed to set key");
		aead->destroy(aead);
		return FALSE;
	}
	if (ctx.decrypt)
	{
		/* the ICV is expected to follow the cipher text */
		chunk_t cipher = chunk_cata("cc", test->cipher, test->icv);
		/* store if the verification of the ICV verification is successful */
		test->success = aead->decrypt(aead, cipher, test->aad, iv,
									  &test->plain);
	}
	else
	{
		if (!aead->encrypt(aead, test->plain, test->aad, iv, &test->cipher))
		{
			DBG1(DBG_APP, "encryption failed");
			aead->destroy(aead);
			return FALSE;
		}
		/* copy ICV from the end of the cipher text */
		test->icv = chunk_alloc(ctx.icvlen / 8);
		test->cipher.len -= test->icv.len;
		memcpy(test->icv.ptr, test->cipher.ptr + test->cipher.len,
			   test->icv.len);
	}
	aead->destroy(aead);
	return TRUE;
}

static bool do_crypt(crypter_t *crypter, test_vector_t *test)
{
	if (ctx.decrypt)
	{
		if (!crypter->decrypt(crypter, test->cipher, test->iv, &test->plain))
		{
			DBG1(DBG_APP, "decryption failed");
			return FALSE;
		}
	}
	else
	{
		if (!crypter->encrypt(crypter, test->plain, test->iv, &test->cipher))
		{
			DBG1(DBG_APP, "encryption failed");
			return FALSE;
		}
	}
	return TRUE;
}

static bool do_test_cbc(test_vector_t *test)
{
	crypter_t *crypter;

	crypter = lib->crypto->create_crypter(lib->crypto, ENCR_AES_CBC,
										  test->key.len);
	if (!crypter)
	{
		DBG1(DBG_APP, "algorithm %N or key length (%d bits) not supported",
			 encryption_algorithm_names, ENCR_AES_CBC, test->key.len * 8);
		return FALSE;
	}
	if (!crypter->set_key(crypter, test->key))
	{
		DBG1(DBG_APP, "failed to set key");
		crypter->destroy(crypter);
		return FALSE;
	}
	if (!do_crypt(crypter, test))
	{
		crypter->destroy(crypter);
		return FALSE;
	}
	crypter->destroy(crypter);
	return TRUE;
}

static bool do_test_mct(test_vector_t *test)
{
	crypter_t *crypter;
	chunk_t prev, *input, *output;
	int i, j;

	crypter = lib->crypto->create_crypter(lib->crypto, ENCR_AES_CBC,
										  test->key.len);
	if (!crypter)
	{
		DBG1(DBG_APP, "algorithm %N or key length (%d bits) not supported",
			 encryption_algorithm_names, ENCR_AES_CBC, test->key.len * 8);
		return FALSE;
	}
	input = ctx.decrypt ? &test->cipher : &test->plain;
	output = ctx.decrypt ? &test->plain : &test->cipher;
	if (crypter->get_block_size(crypter) != input->len)
	{
		DBG1(DBG_APP, "MCT only works for input with a length of one block");
		crypter->destroy(crypter);
		return FALSE;
	}
	prev = chunk_alloca(input->len);
	/* assume initial IV as previous output */
	*output = chunk_clone(test->iv);
	for (i = 0; i < 100; i++)
	{
		if (i > 0)
		{	/* we copied the original lines already */
			fprintf(ctx.out, "COUNT = %d\n", i);
			fprintf(ctx.out, "KEY = %+B\n", &test->key);
			fprintf(ctx.out, "IV = %+B\n", &test->iv);
			fprintf(ctx.out, "%s = %+B\n",
					ctx.decrypt ? "CIPHERTEXT" : "PLAINTEXT", input);
		}
		if (!crypter->set_key(crypter, test->key))
		{
			DBG1(DBG_APP, "failed to set key");
			return FALSE;
		}
		for (j = 0; j < 1000; j++)
		{
			/* store previous output as it is used as input after next */
			memcpy(prev.ptr, output->ptr, prev.len);
			chunk_free(output);
			if (!do_crypt(crypter, test))
			{
				crypter->destroy(crypter);
				return FALSE;
			}
			/* prepare the next IV (our API does not allow incremental calls) */
			if (ctx.decrypt)
			{
				memcpy(test->iv.ptr, input->ptr, test->iv.len);
			}
			else
			{
				memcpy(test->iv.ptr, output->ptr, test->iv.len);
			}
			/* the previous output is the next input */
			memcpy(input->ptr, prev.ptr, input->len);
		}
		fprintf(ctx.out, "%s = %+B\n\n",
				ctx.decrypt ? "PLAINTEXT" : "CIPHERTEXT", output);
		/* derive key for next round */
		switch (test->key.len)
		{
			case 16:
				memxor(test->key.ptr, output->ptr, output->len);
				break;
			case 24:
				memxor(test->key.ptr, prev.ptr + 8, 8);
				memxor(test->key.ptr + 8, output->ptr, output->len);
				break;
			case 32:
				memxor(test->key.ptr, prev.ptr, prev.len);
				memxor(test->key.ptr + prev.len, output->ptr, output->len);
				break;
		}
		/* the current output is used as IV for the next round */
		memcpy(test->iv.ptr, output->ptr, test->iv.len);
	}
	crypter->destroy(crypter);
	/* we return FALSE as we print the output ourselves */
	return FALSE;
}

static bool do_test(test_vector_t *test)
{
	if (ctx.use_gcm)
	{
		return do_test_gcm(test);
	}
	if (ctx.use_mct)
	{
		return do_test_mct(test);
	}
	return do_test_cbc(test);
}

static void usage(FILE *out, char *name)
{
	fprintf(out, "Test AES implementation according to the AES Algorithm Validation Suite (AESAVS)\n");
	fprintf(out, "and the GCM Validation System (GCMVS)\n\n");
	fprintf(out, "%s [OPTIONS]\n\n", name);
	fprintf(out, "Options:\n");
	fprintf(out, "  -h, --help          print this help.\n");
	fprintf(out, "  -d, --debug=LEVEL   set debug level (default 1).\n");
	fprintf(out, "  -m, --mode=MODE     mode to test, either CBC or GCM (default CBC).\n");
	fprintf(out, "  -t, --mct           run Monte Carlo Test (MCT), only for CBC.\n");
	fprintf(out, "  -x, --decrypt       test decryption (not needed for CBC as files contain control directives).\n");
	fprintf(out, "  -i, --in=FILE       request file (default STDIN).\n");
	fprintf(out, "  -o, --out=FILE      response file (default STDOUT).\n");
	fprintf(out, "\n");
}

int main(int argc, char *argv[])
{
	test_vector_t test;

	ctx.in = stdin;
	ctx.out = stdout;

	library_init(NULL, "aes-test");
	atexit(library_deinit);

	while (true)
	{
		struct option long_opts[] = {
			{"help",		no_argument,		NULL,	'h' },
			{"debug",		required_argument,	NULL,	'd' },
			{"mode",		required_argument,	NULL,	'm' },
			{"mct",			no_argument,		NULL,	't' },
			{"decrypt",		no_argument,		NULL,	'x' },
			{"in",			required_argument,	NULL,	'i' },
			{"out",			required_argument,	NULL,	'o' },
			{0,0,0,0 },
		};
		switch (getopt_long(argc, argv, "hd:m:txi:o:", long_opts, NULL))
		{
			case EOF:
				break;
			case 'h':
				usage(stdout, argv[0]);
				return 0;
			case 'd':
				dbg_default_set_level(atoi(optarg));
				continue;
			case 'm':
				if (strcaseeq(optarg, "GCM"))
				{
					ctx.use_gcm = TRUE;
				}
				else if (!strcaseeq(optarg, "CBC"))
				{
					usage(stderr, argv[0]);
					return 1;
				}
				continue;
			case 't':
				ctx.use_mct = TRUE;
				continue;
			case 'x':
				ctx.decrypt = TRUE;
				continue;
			case 'i':
				ctx.in = fopen(optarg, "r");
				if (!ctx.in)
				{
					fprintf(stderr, "failed to open '%s': %s\n", optarg,
							strerror(errno));
					usage(stderr, argv[0]);
					return 1;
				}
				continue;
			case 'o':
				ctx.out = fopen(optarg, "w");
				if (!ctx.out)
				{
					fprintf(stderr, "failed to open '%s': %s\n", optarg,
							strerror(errno));
					usage(stderr, argv[0]);
					return 1;
				}
				continue;
			default:
				usage(stderr, argv[0]);
				return 1;
		}
		break;
	}
	/* TODO: maybe make plugins configurable */
	lib->plugins->load(lib->plugins, PLUGINS);
	lib->plugins->status(lib->plugins, LEVEL_CTRL);

	while (get_next_test_vector(&test))
	{
		if (verify_test_vector(&test))
		{
			if (do_test(&test))
			{
				print_result(&test);
			}
		}
		else
		{
			DBG1(DBG_APP, "test vector with missing data encountered");
		}
		fprintf(ctx.out, "\n");
		test_vector_free(&test);
	}

	if (ctx.in != stdin)
	{
		fclose(ctx.in);
	}
	if (ctx.out != stdout)
	{
		fclose(ctx.out);
	}
	return 0;
}
