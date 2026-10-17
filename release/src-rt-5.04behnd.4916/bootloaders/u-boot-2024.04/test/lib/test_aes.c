// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2019 Philippe Reynes <philippe.reynes@softathome.com>
 *
 * Unit tests for aes functions
 */

#include <common.h>
#include <command.h>
#include <hexdump.h>
#include <rand.h>
#include <uboot_aes.h>
#include <test/lib.h>
#include <test/test.h>
#include <test/ut.h>

#define TEST_AES_ONE_BLOCK		0
#define TEST_AES_CBC_CHAIN		1

struct test_aes_s {
	int key_len;
	int key_exp_len;
	int type;
	int num_block;
};

static struct test_aes_s test_aes[] = {
	{ AES128_KEY_LENGTH, AES128_EXPAND_KEY_LENGTH, TEST_AES_ONE_BLOCK,  1 },
	{ AES128_KEY_LENGTH, AES128_EXPAND_KEY_LENGTH, TEST_AES_CBC_CHAIN, 16 },
	{ AES192_KEY_LENGTH, AES192_EXPAND_KEY_LENGTH, TEST_AES_ONE_BLOCK,  1 },
	{ AES192_KEY_LENGTH, AES192_EXPAND_KEY_LENGTH, TEST_AES_CBC_CHAIN, 16 },
	{ AES256_KEY_LENGTH, AES256_EXPAND_KEY_LENGTH, TEST_AES_ONE_BLOCK,  1 },
	{ AES256_KEY_LENGTH, AES256_EXPAND_KEY_LENGTH, TEST_AES_CBC_CHAIN, 16 },
};

static void rand_buf(u8 *buf, int size)
{
	int i;

	for (i = 0; i < size; i++)
		buf[i] = rand() & 0xff;
}

static int lib_test_aes_one_block(struct unit_test_state *uts, int key_len,
				  u8 *key_exp, u8 *iv, int num_block,
				  u8 *nocipher, u8 *ciphered, u8 *uncipher)
{
	aes_encrypt(key_len, nocipher, key_exp, ciphered);
	aes_decrypt(key_len, ciphered, key_exp, uncipher);

	ut_asserteq_mem(nocipher, uncipher, AES_BLOCK_LENGTH);

	/* corrupt the expanded key */
	key_exp[0]++;
	aes_decrypt(key_len, ciphered, key_exp, uncipher);
	ut_assertf(memcmp(nocipher, uncipher, AES_BLOCK_LENGTH),
		   "nocipher and uncipher should be different\n");

	return 0;
}

static int lib_test_aes_cbc_chain(struct unit_test_state *uts, int key_len,
				  u8 *key_exp, u8 *iv, int num_block,
				  u8 *nocipher, u8 *ciphered, u8 *uncipher)
{
	aes_cbc_encrypt_blocks(key_len, key_exp, iv,
			       nocipher, ciphered, num_block);
	aes_cbc_decrypt_blocks(key_len, key_exp, iv,
			       ciphered, uncipher, num_block);

	ut_asserteq_mem(nocipher, uncipher, num_block * AES_BLOCK_LENGTH);

	/* corrupt the expanded key */
	key_exp[0]++;
	aes_cbc_decrypt_blocks(key_len, key_exp, iv,
			       ciphered, uncipher, num_block);
	ut_assertf(memcmp(nocipher, uncipher, num_block * AES_BLOCK_LENGTH),
		   "nocipher and uncipher should be different\n");

	return 0;
}

static int _lib_test_aes_run(struct unit_test_state *uts, int key_len,
			     int key_exp_len, int type, int num_block)
{
	u8 *key, *key_exp, *iv;
	u8 *nocipher, *ciphered, *uncipher;
	int ret;

	/* Allocate all the buffer */
	key = malloc(key_len);
	key_exp = malloc(key_exp_len);
	iv = malloc(AES_BLOCK_LENGTH);
	nocipher = malloc(num_block * AES_BLOCK_LENGTH);
	ciphered = malloc((num_block + 1) * AES_BLOCK_LENGTH);
	uncipher = malloc((num_block + 1) * AES_BLOCK_LENGTH);

	if (!key || !key_exp || !iv || !nocipher || !ciphered || !uncipher) {
		printf("%s: can't allocate memory\n", __func__);
		ret = -1;
		goto out;
	}

	/* Initialize all buffer */
	rand_buf(key, key_len);
	rand_buf(iv, AES_BLOCK_LENGTH);
	rand_buf(nocipher, num_block * AES_BLOCK_LENGTH);
	memset(ciphered, 0, (num_block + 1) * AES_BLOCK_LENGTH);
	memset(uncipher, 0, (num_block + 1) * AES_BLOCK_LENGTH);

	/* Expand the key */
	aes_expand_key(key, key_len, key_exp);

	/* Encrypt and decrypt */
	switch (type) {
	case TEST_AES_ONE_BLOCK:
		ret = lib_test_aes_one_block(uts, key_len, key_exp, iv,
					     num_block, nocipher,
					     ciphered, uncipher);
		break;
	case TEST_AES_CBC_CHAIN:
		ret = lib_test_aes_cbc_chain(uts, key_len, key_exp, iv,
					     num_block, nocipher,
					     ciphered, uncipher);
		break;
	default:
		printf("%s: unknown type (type=%d)\n", __func__, type);
		ret = -1;
	};

 out:
	/* Free all the data */
	free(key);
	free(key_exp);
	free(iv);
	free(nocipher);
	free(ciphered);
	free(uncipher);

	return ret;
}

static int lib_test_aes_run(struct unit_test_state *uts,
			    struct test_aes_s *test)
{
	int key_len = test->key_len;
	int key_exp_len = test->key_exp_len;
	int type = test->type;
	int num_block = test->num_block;

	return _lib_test_aes_run(uts, key_len, key_exp_len,
				 type, num_block);
}

static int lib_test_aes(struct unit_test_state *uts)
{
	int i, ret = 0;

	for (i = 0; i < ARRAY_SIZE(test_aes); i++) {
		ret = lib_test_aes_run(uts, &test_aes[i]);
		if (ret)
			break;
	}

	return ret;
}

LIB_TEST(lib_test_aes, 0);
