/*
 * Copyright (C) 2008 Thomas Kallenberg
 * Copyright (C) 2008 Martin Willi
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

#include "padlock_aes_crypter.h"
#include <stdio.h>

#define AES_BLOCK_SIZE 16
#define PADLOCK_ALIGN __attribute__ ((__aligned__(16)))

typedef struct private_padlock_aes_crypter_t private_padlock_aes_crypter_t;

/**
 * Private data of padlock_aes_crypter_t
 */
struct private_padlock_aes_crypter_t {

	/**
	 * Public part of this class.
	 */
	padlock_aes_crypter_t public;

	/*
	 * the key
	 */
	chunk_t	key;
};

/**
 * Control word structure to pass to crypt operations
 */
typedef struct {
	u_int __attribute__ ((__packed__))
		rounds:4,
		algo:3,
		keygen:1,
		interm:1,
		encdec:1,
		ksize:2;
	/* microcode needs additional bytes for calculation */
	u_char buf[124];
} cword;

/**
 * Invoke the actual de/encryption
 */
static void padlock_crypt(void *key, void *ctrl, void *src, void *dst,
						  int count, void *iv)
{
	asm volatile(
		"pushl %%eax\n pushl %%ebx\n pushl %%ecx\n"
		"pushl %%edx\n pushl %%esi\n pushl %%edi\n"
		"pushfl\n popfl\n"
		"movl %0, %%eax\n"
		"movl %1, %%ebx\n"
		"movl %2, %%ecx\n"
		"movl %3, %%edx\n"
		"movl %4, %%esi\n"
		"movl %5, %%edi\n"
		"rep\n"
		".byte 0x0f, 0xa7, 0xd0\n"
		"popl %%edi\n popl %%esi\n popl %%edx\n"
		"popl %%ecx\n popl %%ebx\n popl %%eax\n"
		:
		: "m"(iv),"m"(key), "m"(count), "m"(ctrl), "m"(src), "m"(dst)
		: "eax", "ecx", "edx", "esi", "edi");
}

/**
 * Do encryption/decryption operation using Padlock control word
 */
static void crypt(private_padlock_aes_crypter_t *this, char *iv,
				  chunk_t src, chunk_t *dst, bool enc)
{
	cword cword PADLOCK_ALIGN;
	u_char key_aligned[256] PADLOCK_ALIGN;
	u_char iv_aligned[16] PADLOCK_ALIGN;

	memset(&cword, 0, sizeof(cword));

	/* set encryption/decryption flag */
	cword.encdec = enc;
	/* calculate rounds and key size */
	cword.rounds = 10 + (this->key.len - 16) / 4;
	cword.ksize = (this->key.len - 16) / 8;
	/* enable autoalign */
	cword.algo |= 2;

	/* move data to aligned buffers */
	memcpy(iv_aligned, iv, sizeof(iv_aligned));
	memcpy(key_aligned, this->key.ptr, this->key.len);

	*dst = chunk_alloc(src.len);
	padlock_crypt(key_aligned, &cword, src.ptr, dst->ptr,
				  src.len / AES_BLOCK_SIZE, iv_aligned);

	memwipe(key_aligned, sizeof(key_aligned));
}

METHOD(crypter_t, decrypt, bool,
	private_padlock_aes_crypter_t *this, chunk_t data, chunk_t iv, chunk_t *dst)
{
	crypt(this, iv.ptr, data, dst, TRUE);
	return TRUE;
}

METHOD(crypter_t, encrypt, bool,
	private_padlock_aes_crypter_t *this, chunk_t data, chunk_t iv, chunk_t *dst)
{
	crypt(this, iv.ptr, data, dst, FALSE);
	return TRUE;
}

METHOD(crypter_t, get_block_size, size_t,
	private_padlock_aes_crypter_t *this)
{
	return AES_BLOCK_SIZE;
}

METHOD(crypter_t, get_iv_size, size_t,
	private_padlock_aes_crypter_t *this)
{
	return AES_BLOCK_SIZE;
}

METHOD(crypter_t, get_key_size, size_t,
	private_padlock_aes_crypter_t *this)
{
	return this->key.len;
}

METHOD(crypter_t, set_key, bool,
	private_padlock_aes_crypter_t *this, chunk_t key)
{
	memcpy(this->key.ptr, key.ptr, min(key.len, this->key.len));
	return TRUE;
}

METHOD(crypter_t, destroy, void,
	private_padlock_aes_crypter_t *this)
{
	chunk_clear(&this->key);
	free(this);
}

/*
 * Described in header
 */
padlock_aes_crypter_t *padlock_aes_crypter_create(encryption_algorithm_t algo,
												  size_t key_size)
{
	private_padlock_aes_crypter_t *this;

	if (algo != ENCR_AES_CBC)
	{
		return NULL;
	}
	switch (key_size)
	{
		case 0:
			key_size = 16;
			/* FALL */
		case 16:        /* AES 128 */
			break;
		case 24:        /* AES-192 */
		case 32:        /* AES-256 */
			/* These need an expanded key, currently not supported, FALL */
		default:
			return NULL;
	}

	INIT(this,
		.public = {
			.crypter = {
				.encrypt = _encrypt,
				.decrypt = _decrypt,
				.get_block_size = _get_block_size,
				.get_iv_size = _get_iv_size,
				.get_key_size = _get_key_size,
				.set_key = _set_key,
				.destroy = _destroy,
			},
		},
		.key = chunk_alloc(key_size),
	);
	return &this->public;
}
